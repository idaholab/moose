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
  return params;
}

NodalFloodCount::NodalFloodCount(const std::string & name, InputParameters parameters) :
    ElementPostprocessor(name, parameters),
    _threshold(getParam<Real>("threshold")),
    _mesh(_subproblem.mesh()),
    _var_number(_var.number()),
    _region_count(0),
    _pbs(NULL)
{}

void
NodalFloodCount::initialize()
{
  // Get a pointer to the PeriodicBoundaries buried in libMesh
  // TODO: Can we do this in the constructor (i.e. are all objects necessary for this call in existance during ctor?)
  _pbs = dynamic_cast<FEProblem *>(&_subproblem)->getNonlinearSystem().dofMap().get_periodic_boundaries();

  // Clear the bubble marking map
  _bubble_map.clear();

  // Reset the packed data structure
  _packed_data.clear();

  // Reset the ownership structure
  _region_to_var_idx.clear();

  // Reset the region counter
  _region_count = 0;

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

    flood(node, 0, -1);
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
  return _bubble_sets.size();
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
NodalFloodCount::pack(std::vector<unsigned int> & packed_data) const
{
  // Don't repack the data if it's already packed - we might lose data that was updated
  // or stored into the packed_data that is not available in the local thread
  if (!packed_data.empty())
    return;

  std::vector<std::set<unsigned int> > data(_region_count+1);
  unsigned int n_periodic_nodes = 0;

  {
    std::map<unsigned int, int>::const_iterator end = _bubble_map.end();
    // Reorganize the data by values
    for (std::map<unsigned int, int>::const_iterator it = _bubble_map.begin(); it != end; ++it)
      data[(it->second)].insert(it->first);

    // Append our periodic neighbor nodes to the data structure before packing
    n_periodic_nodes = appendPeriodicNeighborNodes(data);

    mooseAssert(_region_count+1 == data.size(), "Error in packing data");
  }

  {
    /**
     * The size of the packed data structure should be the sum of all of the following:
     * total number of marked nodes
     * the owning variable index for the current bubble
     * inserted periodic neighbor information
     * the number of unique bubbles.
     *
     * This first resize will be an overesitmate because we aren't going to store nodes
     * marked with zeros (outside of bubbles).
     *
     * We will pack the data into a series of groups representing each unique bubble
     * the nodes for each group will be proceeded by the number of nodes in that group
     * [ <i_nodes> <var_idx> <n_0> <n_1> ... <n_i> <j_nodes> <var_idx> <n_0> <n_1> ... <n_j> ]
     */
    packed_data.resize(_bubble_map.size() + n_periodic_nodes + _region_count*2 /* <- owning idx + number of unique bubbles */);

    // Now pack it up
    unsigned int current_idx = 0;

    // Note: The zeroth "region" is everything outside of a bubble - we don't want to put
    // that into our packed data structure so start at 1 here!
    for (unsigned int i=1 /* Yes - start at 1 */; i<=_region_count; ++i)
    {
      packed_data[current_idx++] = data[i].size();                      // The number of nodes in the current bubble
      packed_data[current_idx++] = _region_to_var_idx[i-1];             // The variable owning this bubble
      std::set<unsigned int>::iterator end = data[i].end();
      for (std::set<unsigned int>::iterator it = data[i].begin(); it != end; ++it)
        packed_data[current_idx++] = *it;                               // The individual node ids
    }

    // Resize the structure to it's actual final size
    packed_data.resize(current_idx);
  }
}

void
NodalFloodCount::unpack(const std::vector<unsigned int> & packed_data)
{
  bool next_set = true;
  unsigned int curr_set_length=0;
  std::set<unsigned int> curr_set;
  unsigned int curr_region;

  _bubble_sets.clear();
  for (unsigned int i=0; i<packed_data.size(); ++i)
  {
    if (next_set)
    {
      if (i > 0)
      {
        _bubble_sets.push_back(BubbleData(curr_set, curr_region));
        curr_set.clear();
      }

      // Get the length of the next set
      curr_set_length = packed_data[i];
      // Also get the owning variable idx.
      // Note: We are intentionally advancing "i" here too!
      curr_region = packed_data[++i];
    }
    else
    {
      // unpack each bubble
      curr_set.insert(packed_data[i]);
      --curr_set_length;
    }

    next_set = !(curr_set_length);
  }
  _bubble_sets.push_back(BubbleData(curr_set, curr_region));

  mooseAssert(curr_set_length == 0, "Error in unpacking data");
}

void
NodalFloodCount::mergeSets()
{
  std::set<unsigned int> set_union;
  bool set_merged;

  /**
   * This loop will continue as long as sets of nodes are merged
   * creating "new" sets.  There may be other ways to optimize this loop
   * but it should not be a bottleneck in practice.
   */
  do
  {
    set_merged = false;
    std::list<BubbleData>::iterator end = _bubble_sets.end();
    for (std::list<BubbleData>::iterator it1 = _bubble_sets.begin(); it1 != end; ++it1)
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
          it1->_nodes = set_union;
          _bubble_sets.erase(it2++);
          set_merged = true;
        }
        else
          ++it2;
      }
    }
  } while (set_merged);

  /**
   * Finally update the original bubble map with field data from the merged sets
   */
  unsigned int counter = 1;
  for (std::list<BubbleData>::iterator it1 = _bubble_sets.begin(); it1 != _bubble_sets.end(); ++it1)
  {
    for (std::set<unsigned int>::iterator it2 = it1->_nodes.begin(); it2 != it1->_nodes.end(); ++it2)
    {
      _bubble_map[*it2] = counter;
    }
    ++counter;
  }
}

void
NodalFloodCount::flood(const Node *node, unsigned int region, int current_idx)
{
  if (node == NULL)
    return;

  unsigned int node_id = node->id();

  // Has this node already been marked? - if so move along
  if (_bubble_map.find(node_id) != _bubble_map.end())
    return;

  /**
   * This node hasn't been marked - check to see if it in a bubble.
   * If current_idx is set (>= zero) then we are in the process of marking a bubble.
   */
  if (current_idx >= 0)
  {
    if (_vars[current_idx]->getNodalValue(*node) < _threshold)
    {
      // No - mark and return
      _bubble_map[node_id] = 0;
      return;
    }
  }
  else  // If current_idx is not set (< zero), then we can look for the start of a new bubble
  {
    for (unsigned int i=0; i<_vars.size(); ++i)
      if (_vars[i]->getNodalValue(*node) >= _threshold)
      {
        current_idx = i;
        break;
      }
    // Did we still fail to find any new bubbles?
    if (current_idx < 0)
    {
      _bubble_map[node_id] = 0;
      return;
    }
  }

  std::vector< const Node * > neighbors;
  MeshTools::find_nodal_neighbors(_mesh._mesh, *node, _nodes_to_elem_map, neighbors);
  // Important!  If this node doesn't have any neighbors (i.e. floating node in the center
  // of an element) we need to just unmark it for now since it can't be easiliy flooded
  if (neighbors.size() == 0)
  {
    _bubble_map[node_id] = 0;
    return;
  }

  // Yay! A bubble -> Mark it! (If region is zero, that signifies that this is a new bubble)
  if (region == 0)
  {
    _bubble_map[node_id] = ++_region_count;
    _region_to_var_idx.push_back(current_idx);
  }
  else
    _bubble_map[node_id] = region;

  // Flood neighboring nodes that are also above this threshold with recursion
  for (unsigned int i=0; i<neighbors.size(); ++i)
  {
    // Only recurse on nodes this processor owns
    if (!region || isNodeValueValid(neighbors[i]->id()))
    {
      flood(neighbors[i], _bubble_map[node_id], current_idx);
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
