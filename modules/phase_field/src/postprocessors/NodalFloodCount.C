/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "NodalFloodCount.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"

#include "NonlinearSystem.h"
#include "FEProblem.h"


//libMesh includes
#include "dof_map.h"
#include "mesh_tools.h"
#include "periodic_boundaries.h"
#include "point_locator_base.h"

#include <algorithm>
#include <limits>

template<>
InputParameters validParams<NodalFloodCount>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  params.addParam<Real>("threshold", 0.5, "The threshold value of the bubble boundary");
  params.addParam<std::string>("elem_avg_value", "If supplied, will be used to find the scaled threshold of the bubble edges");
  params.addParam<bool>("use_single_map", true, "Determine whether information is tracked per coupled variable or consolidated into one (default: true)");
  return params;
}

NodalFloodCount::NodalFloodCount(const std::string & name, InputParameters parameters) :
    ElementPostprocessor(name, parameters),
    _threshold(getParam<Real>("threshold")),
    _mesh(_subproblem.mesh()),
    _var_number(_var.number()),
    _single_map_mode(getParam<bool>("use_single_map")),
    _maps_size(_single_map_mode ? 1 : _vars.size()),
    _pbs(NULL),
    _element_average_value(parameters.isParamValid("elem_avg_value") ? getPostprocessorValue("elem_avg_value") : _real_zero)
{
  // Size the data structures to hold the correct number of maps
  _bubble_maps.resize(_maps_size);
  _bubble_sets.resize(_maps_size);
  _region_counts.resize(_maps_size);
}

void
NodalFloodCount::initialize()
{
  // Get a pointer to the PeriodicBoundaries buried in libMesh
  // TODO: Can we do this in the constructor (i.e. are all objects necessary for this call in existance during ctor?)
  _pbs = dynamic_cast<FEProblem *>(&_subproblem)->getNonlinearSystem().dofMap().get_periodic_boundaries();

  // Clear the bubble marking maps and region counters
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
  {
    _bubble_maps[map_num].clear();
    _region_counts[map_num] = 0;
  }

  // Clear the visited nodes map
  _nodes_visited.clear();

  // Clear the packed data structure
  _packed_data.clear();

  // Reset the ownership structure
  _region_to_var_idx.clear();

  // Build a new node to element map
  _nodes_to_elem_map.clear();
  MeshTools::build_nodes_to_elem_map(_mesh._mesh, _nodes_to_elem_map);

  // TODO: We might only need to build this once if adaptivity is turned off
  _mesh.buildPeriodicNodeMap(_periodic_node_map, _var_number, _pbs);
}

void
NodalFloodCount::execute()
{
  unsigned int n_nodes = _current_elem->n_vertices();
  for (unsigned int i=0; i < n_nodes; ++i)
  {
    const Node *node = _current_elem->get_node(i);

    flood(node, std::map<unsigned int, unsigned int>());
  }
}

void
NodalFloodCount::finalize()
{
  // Exchange data in parallel
  pack(_packed_data);
  Parallel::allgather(_packed_data, false);
  unpack(_packed_data);

  mergeSets();
}

Real
NodalFloodCount::getValue()
{
  unsigned int count = 0;

  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
    count += _bubble_sets[map_num].size();

  return count;
}

Real
NodalFloodCount::getNodeValue(unsigned int node_id, unsigned int var_idx) const
{
  std::map<unsigned int, int>::const_iterator node_it = _bubble_maps[var_idx].find(node_id);

  if (node_it != _bubble_maps[var_idx].end())
    return node_it->second;
  else
    return 0;
}

void
NodalFloodCount::threadJoin(const UserObject &y)
{
   const NodalFloodCount & pps = dynamic_cast<const NodalFloodCount &>(y);

   // Pack up the data on both of the threads
   pack(_packed_data);

   std::vector<unsigned int> pps_packed_data;
   pps.pack(pps_packed_data);

   // Append the packed data structures together
   std::copy(pps_packed_data.begin(), pps_packed_data.end(), std::back_inserter(_packed_data));
}


void
NodalFloodCount::pack(std::vector<unsigned int> & packed_data, bool merge_periodic_info) const
{
  /**
   * Don't repack the data if it's already packed - we might lose data that was updated
   * or stored into the packed_data that is not available in the local thread.
   * This happens when we call threadJoin which does not unpack the data on the local thread.
   */
  if (!packed_data.empty())
    return;

  /**
   * We need a data structure that reorganizes the region markings into sets so that we can pack them up
   * in a form to marshall them between processors.  The set of nodes are stored by map_num, region_num.
   **/
  std::vector<std::vector<std::set<unsigned int> > > data(_maps_size);

  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
  {
    data[map_num].resize(_region_counts[map_num]+1);

    unsigned int n_periodic_nodes = 0;
    {
      std::map<unsigned int, int>::const_iterator end = _bubble_maps[map_num].end();
      // Reorganize the data by values

      for (std::map<unsigned int, int>::const_iterator it = _bubble_maps[map_num].begin(); it != end; ++it)
        data[map_num][(it->second)].insert(it->first);

      // Append our periodic neighbor nodes to the data structure before packing
      if (merge_periodic_info)
        n_periodic_nodes = appendPeriodicNeighborNodes(data[map_num]);

      mooseAssert(_region_counts[map_num]+1 == data[map_num].size(), "Error in packing data");
    }

    {
      /**
       * The size of the packed data structure should be the sum of all of the following:
       * total number of marked nodes
       * the owning variable index for the current bubble
       * inserted periodic neighbor information
       * the number of unique bubbles.
       *
       * We will pack the data into a series of groups representing each unique bubble
       * the nodes for each group will be proceeded by the number of nodes in that group
       * [ <i_nodes> <var_idx> <n_0> <n_1> ... <n_i> <j_nodes> <var_idx> <n_0> <n_1> ... <n_j> ]
       */

      // Note the _region_counts[mar_num]*2 takes into account the number of nodes and the variable index for each region
      std::vector<unsigned int> partial_packed_data(_bubble_maps[map_num].size() + n_periodic_nodes + _region_counts[map_num]*2);

      // Now pack it up
      unsigned int current_idx = 0;

      mooseAssert(data[map_num][0].empty(), "We have nodes marked with zeros - something is not correct");
      // Note: The zeroth "region" is everything outside of a bubble - we don't want to put
      // that into our packed data structure so start at 1 here!
      for (unsigned int i=1 /* Yes - start at 1 */; i<=_region_counts[map_num]; ++i)
      {
        partial_packed_data[current_idx++] = data[map_num][i].size();     // The number of nodes in the current region

        if (_single_map_mode)
        {
          mooseAssert(i-1 < _region_to_var_idx.size(), "Index out of bounds in NodalFloodCounter");
          partial_packed_data[current_idx++] = _region_to_var_idx[i-1];   // The variable owning this bubble
        }
        else
          partial_packed_data[current_idx++] = map_num;                   // The variable owning this bubble

        std::set<unsigned int>::iterator end = data[map_num][i].end();
        for (std::set<unsigned int>::iterator it = data[map_num][i].begin(); it != end; ++it)
          partial_packed_data[current_idx++] = *it;                       // The individual node ids
      }

      packed_data.insert(packed_data.end(), partial_packed_data.begin(), partial_packed_data.end());
    }
  }
}

void
NodalFloodCount::unpack(const std::vector<unsigned int> & packed_data)
{
  bool start_next_set = true;
  bool has_data_to_save = false;

  unsigned int curr_set_length=0;
  std::set<unsigned int> curr_set;
  unsigned int curr_var_idx = std::numeric_limits<unsigned int>::max();

  for (unsigned int i=0; i<_maps_size; ++i)
    _bubble_sets[i].clear();

  _region_to_var_idx.clear();
  for (unsigned int i=0; i<packed_data.size(); ++i)
  {
    if (start_next_set)
    {
      if (has_data_to_save)
      {
        // See Note at the bottom of this routine
        _bubble_sets[_single_map_mode ? 0 : curr_var_idx].push_back(BubbleData(curr_set, curr_var_idx));
        _region_to_var_idx.push_back(curr_var_idx);
        curr_set.clear();
      }

      // Get the length of the next set
      curr_set_length = packed_data[i];
      // Also get the owning variable idx.
      // Note: We are intentionally advancing "i" here too!
      curr_var_idx = packed_data[++i];
    }
    else
    {
      // unpack each bubble
      curr_set.insert(packed_data[i]);
      --curr_set_length;
    }

    start_next_set = !(curr_set_length);
    has_data_to_save = true;
  }

  /**
   * Note: In multi-map mode the var_idx information stored inside of BubbleData is redundant with
   * the outer index of the _bubble_sets data-structure.  We need this information for single-map
   * mode when we have multiple variables coupled in.
   */
  if (has_data_to_save)
  {
    _bubble_sets[_single_map_mode ? 0 : curr_var_idx].push_back(BubbleData(curr_set, curr_var_idx));
    _region_to_var_idx.push_back(curr_var_idx);
  }

  mooseAssert(curr_set_length == 0, "Error in unpacking data");
}

void
NodalFloodCount::mergeSets()
{
  std::set<unsigned int> set_union;

  /**
   * This loop will continue as long as sets of nodes are merged
   * creating "new" sets.  There may be other ways to optimize this loop
   * but it should not be a bottleneck in practice.
   */
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
  {
    bool set_merged;
    do
    {
      set_merged = false;
      std::list<BubbleData>::iterator end = _bubble_sets[map_num].end();
      for (std::list<BubbleData>::iterator it1 = _bubble_sets[map_num].begin(); it1 != end; ++it1)
      {
        std::list<BubbleData>::iterator it2 = it1;
        ++it2; // Don't compare this set with itself - advance the iterator to the next set
        while (it2 != end)
        {
          set_union.clear();
          std::set_union(it1->_nodes.begin(), it1->_nodes.end(), it2->_nodes.begin(), it2->_nodes.end(),
                         std::inserter(set_union, set_union.end()));

          if (set_union.size() < it1->_nodes.size() + it2->_nodes.size())
          {
            // Merge these two sets and remove the duplicate set
            it1->_nodes = set_union;
            _bubble_sets[map_num].erase(it2++);
            set_merged = true;
          }
          else
            ++it2;
        }
      }
    } while (set_merged);
  }

  // This variable is only relevant in single map mode
  _region_to_var_idx.resize(_bubble_sets[0].size());


  // Finally update the original bubble map with field data from the merged sets
  for (unsigned int map_num=0; map_num < _maps_size; ++map_num)
  {
    unsigned int counter = 1;
    for (std::list<BubbleData>::iterator it1 = _bubble_sets[map_num].begin(); it1 != _bubble_sets[map_num].end(); ++it1)
    {
      for (std::set<unsigned int>::iterator it2 = it1->_nodes.begin(); it2 != it1->_nodes.end(); ++it2)
        _bubble_maps[map_num][*it2] = counter;

      _region_to_var_idx[counter-1] = it1->_var_idx;
      ++counter;
    }

    _region_counts[map_num] = counter-1;
  }
}

void
NodalFloodCount::flood(const Node *node, std::map<unsigned int, unsigned int> live_region)
{
  if (node == NULL)
    return;
  unsigned int node_id = node->id();

  // Has this node already been marked? - if so move along
  if (_nodes_visited.find(node_id) != _nodes_visited.end())
    return;

  /**
   * In single map mode, we can only have one live region at time even though the possibility
   * exists that multiple variables might actually be active in a region.  We'll favor the active
   * region first before looking at values of the rest of the variables.
   */
  bool already_marked = false;
  if (_single_map_mode && !live_region.empty())
  {
    mooseAssert(live_region.size() == 1, "More than one active region in single_map_mode");

    if (_vars[live_region.begin()->first]->getNodalValue(*node) >= _element_average_value + _threshold)
    {
      _bubble_maps[0][node_id] = live_region.begin()->second;
      already_marked = true;
    }
    /**
     * If we fail to meet the threshold here, then unmarking of the live region will happen properly
     * during the search for another variable in the loop below
     */
  }

  // Loop over all the variables and mark appropriately
  if (!already_marked)
  {
    for (unsigned int var_num=0; var_num<_vars.size(); ++var_num)
    {
      unsigned int map_num = _single_map_mode ? 0 : var_num;

      if (_vars[var_num]->getNodalValue(*node) >= _element_average_value + _threshold)
      {
        // TODO: There are still a few issues with overlapping variables and live_regions
        // that need to be sorted out

        // This variable is in a region at this node
        if (live_region.find(var_num) == live_region.end())     // Is there an live region for this variable?
        {
          live_region[var_num] = ++_region_counts[map_num];     // If not, then this is a new region!
          _region_to_var_idx.push_back(var_num);                // save the variable index that owns this region
        }

        _bubble_maps[map_num][node_id] = live_region[var_num];  // Mark it
      }
      else
      {
        // This variable is NOT in a region at this node
        live_region.erase(var_num);
      }
    }
  }

  // Mark the node as visited
  _nodes_visited[node_id] = true;

  // If no variables are in a live region then we can avoid the recursion step
  if (live_region.empty())
    return;

  std::vector< const Node * > neighbors;
  MeshTools::find_nodal_neighbors(_mesh._mesh, *node, _nodes_to_elem_map, neighbors);

  // Flood neighboring nodes that are also above this threshold with recursion
  for (unsigned int i=0; i<neighbors.size(); ++i)
  {
    // Only recurse on nodes this processor owns
    if (isNodeValueValid(neighbors[i]->id()))
    {
      flood(neighbors[i], live_region);
    }
  }
}

bool
NodalFloodCount::isNodeValueValid(unsigned int node_id) const
{
  for (unsigned int j=0; j < _nodes_to_elem_map[node_id].size(); ++j)
    if (_nodes_to_elem_map[node_id][j]->processor_id() == libMesh::processor_id())
      return true;

  return false;
}

unsigned int
NodalFloodCount::appendPeriodicNeighborNodes(std::vector<std::set<unsigned int> > & data) const
{
  unsigned int inserted_counts = 0;

  /**
   * Now we will append our periodic neighbor information.  We treat the periodic neighbor nodes
   * much like we do ghosted nodes in a multi-processor setting.  If a bubble is sitting on a
   * periodic boundary we will simply add those periodic neighbors to the appropriate bubble
   * before packing up the data
   */
  for (unsigned int i = 0; i < data.size(); ++i)
  {
    std::set<unsigned int> periodic_neighbors;
    std::set<unsigned int> merged_sets;

    for (std::set<unsigned int>::iterator s_it = data[i].begin(); s_it != data[i].end(); ++s_it)
    {
      std::pair<std::multimap<unsigned int, unsigned int>::const_iterator, std::multimap<unsigned int, unsigned int>::const_iterator> iters =
        _periodic_node_map.equal_range(*s_it);
      for (std::multimap<unsigned int, unsigned int>::const_iterator it = iters.first; it != iters.second; ++it)
        periodic_neighbors.insert(it->second);
    }

    // Now that we have all of the periodic_neighbors in our temporary set we need to union it together with the original set
    // for the current bubble
    std::set_union(data[i].begin(), data[i].end(), periodic_neighbors.begin(), periodic_neighbors.end(), std::inserter(merged_sets, merged_sets.end()));

    inserted_counts += merged_sets.size() - data[i].size();
    // save the updated set back into our datastructure
    data[i] = merged_sets;
  }
  return inserted_counts;
}
