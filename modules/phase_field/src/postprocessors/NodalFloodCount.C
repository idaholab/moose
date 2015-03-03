/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "NodalFloodCount.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseUtils.h"

#include "NonlinearSystem.h"
#include "FEProblem.h"

//libMesh includes
#include "libmesh/dof_map.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/point_locator_base.h"

#include <algorithm>
#include <limits>

template<>
InputParameters validParams<NodalFloodCount>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredCoupledVar("variable", "Ths variable(s) for which to find connected regions of interests, i.e. \"bubbles\".");
  params.addParam<Real>("threshold", 0.5, "The threshold value for which a new bubble may be started");
  params.addParam<Real>("connecting_threshold", "The threshold for which an existing bubble may be extended (defaults to \"threshold\")");
  params.addParam<PostprocessorName>("elem_avg_value", "If supplied, will be used to find the scaled threshold of the bubble edges");
  params.addParam<bool>("use_single_map", true, "Determine whether information is tracked per coupled variable or consolidated into one (default: true)");
  params.addParam<bool>("condense_map_info", false, "Determines whether we condense all the node values when in multimap mode (default: false)");
  params.addParam<bool>("use_global_numbering", false, "Determine whether or not global numbers are used to label bubbles on multiple maps (default: false)");
  params.addParam<bool>("enable_var_coloring", false, "Instruct the UO to populate the variable index map.");
  params.addParam<bool>("use_less_than_threshold_comparison", true, "Controls whether bubbles are defined to be less than or greater than the threshold value.");
  params.addParam<FileName>("bubble_volume_file", "An optional file name where bubble volumes can be output.");
  params.addParam<bool>("track_memory_usage", false, "Calculate memory usage");
  params.addParam<bool>("compute_boundary_intersecting_volume", false, "If true, also compute the (normalized) volume of bubbles which intersect the boundary");
  return params;
}

NodalFloodCount::NodalFloodCount(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    Coupleable(parameters, false),
    MooseVariableDependencyInterface(),
    ZeroInterface(parameters),
    _vars(getCoupledMooseVars()),
    _threshold(getParam<Real>("threshold")),
    _connecting_threshold(isParamValid("connecting_threshold") ? getParam<Real>("connecting_threshold") : getParam<Real>("threshold")),
    _mesh(_subproblem.mesh()),
    _var_number(_vars[0]->number()),
    _single_map_mode(getParam<bool>("use_single_map")),
    _condense_map_info(getParam<bool>("condense_map_info")),
    _global_numbering(getParam<bool>("use_global_numbering")),
    _var_index_mode(getParam<bool>("enable_var_coloring")),
    _use_less_than_threshold_comparison(getParam<bool>("use_less_than_threshold_comparison")),
    _maps_size(_single_map_mode ? 1 : _vars.size()),
    _pbs(NULL),
    _element_average_value(parameters.isParamValid("elem_avg_value") ? getPostprocessorValue("elem_avg_value") : _real_zero),
    _track_memory(getParam<bool>("track_memory_usage")),
    _compute_boundary_intersecting_volume(getParam<bool>("compute_boundary_intersecting_volume"))
{
  // Size the data structures to hold the correct number of maps
  _bubble_maps.resize(_maps_size);
  _bubble_sets.resize(_maps_size);
  _region_counts.resize(_maps_size);
  _region_offsets.resize(_maps_size);

  if (_var_index_mode)
    _var_index_maps.resize(_maps_size);

  // This map is always size to the number of variables
  _nodes_visited.resize(_vars.size());
}

NodalFloodCount::~NodalFloodCount()
{
}

void
NodalFloodCount::initialize()
{
  // Get a pointer to the PeriodicBoundaries buried in libMesh
  // TODO: Can we do this in the constructor (i.e. are all objects necessary for this call in existance during ctor?)
  _pbs = dynamic_cast<FEProblem *>(&_subproblem)->getNonlinearSystem().dofMap().get_periodic_boundaries();

  // Clear the bubble marking maps and region counters and other data structures
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    _bubble_maps[map_num].clear();
    _bubble_sets[map_num].clear();
    _region_counts[map_num] = 0;
    _nodes_visited[map_num].clear();

    if (_var_index_mode)
      _var_index_maps[map_num].clear();
  }

  // TODO: use iterator
  for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
    _nodes_visited[var_num].clear();

  // Clear the packed data structure
  _packed_data.clear();

  // Reset the ownership structure
  _region_to_var_idx.clear();

  // Build a new node to element map
  _nodes_to_elem_map.clear();
  MeshTools::build_nodes_to_elem_map(_mesh.getMesh(), _nodes_to_elem_map);

  // TODO: We might only need to build this once if adaptivity is turned off
  _mesh.buildPeriodicNodeMap(_periodic_node_map, _var_number, _pbs);

  // Calculate the thresholds for this iteration
  _step_threshold = _element_average_value + _threshold;
  _step_connecting_threshold = _element_average_value + _connecting_threshold;

  _all_bubble_volumes.clear();

  _bytes_used = 0;
}

void
NodalFloodCount::execute()
{
  const MeshBase::element_iterator end = _mesh.getMesh().active_local_elements_end();
  for (MeshBase::element_iterator el = _mesh.getMesh().active_local_elements_begin(); el != end; ++el)
  {
    Elem *current_elem = *el;
    unsigned int n_nodes = current_elem->n_vertices();
    for (unsigned int i = 0; i < n_nodes; ++i)
    {
      const Node *node = current_elem->get_node(i);

      for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
        flood(node, var_num, 0);
    }
  }
}

void
NodalFloodCount::finalize()
{
  // Exchange data in parallel
  pack(_packed_data);
  _communicator.allgather(_packed_data, false);
  unpack(_packed_data);

  mergeSets();

  // Populate _bubble_maps and _var_index_maps
  updateFieldInfo();

  // Update the region offsets so we can get unique bubble numbers in multimap mode
  updateRegionOffsets();

  // Calculate and out output bubble volume data
  if (_pars.isParamValid("bubble_volume_file"))
  {
    calculateBubbleVolumes();
    std::vector<Real> data;
    data.reserve(_all_bubble_volumes.size() + _total_volume_intersecting_boundary.size() + 2);

    // Insert the current timestep and the simulation time into the data vector
    data.push_back(_fe_problem.timeStep());
    data.push_back(_fe_problem.time());

    // Insert the (sorted) bubble volumes into the data vector
    data.insert(data.end(), _all_bubble_volumes.begin(), _all_bubble_volumes.end());

    // If we are computing the boundary-intersecting volumes, insert
    // those numbers into the normalized boundary-intersecting bubble
    // volumes into the data vector.
    if (_compute_boundary_intersecting_volume)
      data.insert(data.end(), _total_volume_intersecting_boundary.begin(), _total_volume_intersecting_boundary.end());

    // Finally, write the file
    writeCSVFile(getParam<FileName>("bubble_volume_file"), data);
  }

  // Calculate memory usage
  if (_track_memory)
  {
    _bytes_used += calculateUsage();
    _communicator.sum(_bytes_used);
    formatBytesUsed();
  }
}

Real
NodalFloodCount::getValue()
{
  unsigned int count = 0;

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    count += _bubble_sets[map_num].size();

  return count;
}


Real
NodalFloodCount::getNodalValue(dof_id_type node_id, unsigned int var_idx, bool show_var_coloring) const
{
  mooseAssert(var_idx < _maps_size, "Index out of range");
  mooseAssert(!show_var_coloring || _var_index_mode, "Cannot use \"show_var_coloring\" without \"enable_var_coloring\"");

  if (show_var_coloring)
  {
    std::map<dof_id_type, int>::const_iterator node_it = _var_index_maps[var_idx].find(node_id);

    if (node_it != _var_index_maps[var_idx].end())
      return node_it->second;
    else
      return 0;
  }
  else
  {
    std::map<dof_id_type, int>::const_iterator node_it = _bubble_maps[var_idx].find(node_id);

    if (node_it != _bubble_maps[var_idx].end())
      return node_it->second + _region_offsets[var_idx];
    else
      return 0;
  }
}

Real
NodalFloodCount::getElementalValue(dof_id_type /*element_id*/) const
{
  mooseDoOnce(mooseWarning("Method not implemented"));
  return 0;
}

const std::vector<std::pair<unsigned int, unsigned int> > &
NodalFloodCount::getNodalValues(dof_id_type /*node_id*/) const
{
  mooseDoOnce(mooseWarning("Method not implemented"));
  return _empty;
}

std::vector<std::vector<std::pair<unsigned int, unsigned int> > >
NodalFloodCount::getElementalValues(dof_id_type /*elem_id*/) const
{
  std::vector<std::vector<std::pair<unsigned int, unsigned int> > > empty;

  mooseDoOnce(mooseWarning("Method not implemented"));
  return empty;
}

/*
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

  // Calculate thread Memory Usage
  if (_track_memory)
  _bytes_used += pps.calculateUsage();
  }
*/

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
  std::vector<std::vector<std::set<dof_id_type> > > data(_maps_size);

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    data[map_num].resize(_region_counts[map_num]+1);

    unsigned int n_periodic_nodes = 0;
    {
      std::map<dof_id_type, int>::const_iterator end = _bubble_maps[map_num].end();
      // Reorganize the data by values

      for (std::map<dof_id_type, int>::const_iterator it = _bubble_maps[map_num].begin(); it != end; ++it)
        data[map_num][(it->second)].insert(it->first);

      // Append our periodic neighbor nodes to the data structure before packing
      if (merge_periodic_info)
        for (std::vector<std::set<dof_id_type> >::iterator it = data[map_num].begin(); it != data[map_num].end(); ++it)
          n_periodic_nodes += appendPeriodicNeighborNodes(*it);

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
      std::vector<dof_id_type> partial_packed_data(_bubble_maps[map_num].size() + n_periodic_nodes + _region_counts[map_num]*2);

      // Now pack it up
      unsigned int current_idx = 0;

      mooseAssert(data[map_num][0].empty(), "We have nodes marked with zeros - something is not correct");
      // Note: The zeroth "region" is everything outside of a bubble - we don't want to put
      // that into our packed data structure so start at 1 here!
      for (unsigned int i = 1 /* Yes - start at 1 */; i <= _region_counts[map_num]; ++i)
      {
        partial_packed_data[current_idx++] = data[map_num][i].size();     // The number of nodes in the current region

        if (_single_map_mode)
        {
          mooseAssert(i-1 < _region_to_var_idx.size(), "Index out of bounds in NodalFloodCounter");
          partial_packed_data[current_idx++] = _region_to_var_idx[i-1];   // The variable owning this bubble
        }
        else
          partial_packed_data[current_idx++] = map_num;                   // The variable owning this bubble

        std::set<dof_id_type>::iterator end = data[map_num][i].end();
        for (std::set<dof_id_type>::iterator it = data[map_num][i].begin(); it != end; ++it)
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

  unsigned int curr_set_length = 0;
  std::set<dof_id_type> curr_set;
  unsigned int curr_var_idx = std::numeric_limits<unsigned int>::max();

  _region_to_var_idx.clear();
  for (unsigned int i = 0; i < packed_data.size(); ++i)
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
  Moose::perf_log.push("mergeSets()", "NodalFloodCount");
  std::set<dof_id_type> set_union;
  std::insert_iterator<std::set<dof_id_type> > set_union_inserter(set_union, set_union.begin());

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    std::list<BubbleData>::iterator end = _bubble_sets[map_num].end();
    for (std::list<BubbleData>::iterator it1 = _bubble_sets[map_num].begin(); it1 != end; /* No increment */)
    {
      bool need_it1_increment = true;

      for (std::list<BubbleData>::iterator it2 = it1; it2 != end; ++it2)
      {
        if (it1 != it2 &&                       // Make sure that these iterators aren't pointing at the same set
            it1->_var_idx == it2->_var_idx &&   // and that the sets have matching variable indices...
                                                // then See if they overlap
            setsIntersect(it1->_nodes.begin(), it1->_nodes.end(), it2->_nodes.begin(), it2->_nodes.end()))
        {
          // Merge these two sets and remove the duplicate set
          set_union.clear();
          std::set_union(it1->_nodes.begin(), it1->_nodes.end(), it2->_nodes.begin(), it2->_nodes.end(), set_union_inserter);

          // Put the merged set in the latter iterator so that we'll compare earlier sets to it again
          it2->_nodes = set_union;
          _bubble_sets[map_num].erase(it1++);

          // don't increment the outer loop since we just deleted it incremented
          need_it1_increment = false;
          // break out of the inner loop and move on
          break;
        }
      }

      if (need_it1_increment)
        ++it1;
    }
  }
  Moose::perf_log.pop("mergeSets()", "NodalFloodCount");
}

void
NodalFloodCount::updateFieldInfo()
{
  // This variable is only relevant in single map mode
  _region_to_var_idx.resize(_bubble_sets[0].size());

  // Finally update the original bubble map with field data from the merged sets
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    unsigned int counter = 1;
    for (std::list<BubbleData>::iterator it1 = _bubble_sets[map_num].begin(); it1 != _bubble_sets[map_num].end(); ++it1)
    {
      for (std::set<dof_id_type>::iterator it2 = it1->_nodes.begin(); it2 != it1->_nodes.end(); ++it2)
      {
        // Color the bubble map with a unique region
        _bubble_maps[map_num][*it2] = counter;
        if (_var_index_mode)
          _var_index_maps[map_num][*it2] = it1->_var_idx;
      }

      if (_single_map_mode)
        _region_to_var_idx[counter-1] = it1->_var_idx;
      ++counter;
    }

    _region_counts[map_num] = counter-1;
  }
}

void
NodalFloodCount::flood(const Node *node, int current_idx, unsigned int live_region)
{
  if (node == NULL)
    return;
  dof_id_type node_id = node->id();

  // Has this node already been marked? - if so move along
  if (_nodes_visited[current_idx].find(node_id) != _nodes_visited[current_idx].end())
    return;

  // Mark this node as visited
  _nodes_visited[current_idx][node_id] = true;

  // Determing which threshold to use based on whether this is an established region
  Real threshold = (live_region ? _step_connecting_threshold : _step_threshold);

  // Get the value of the current variable at the current node
  Number nodal_val = _vars[current_idx]->getNodalValue(*node);

  // This node hasn't been marked, is it in a bubble?  We must respect
  // the user-selected value of _use_less_than_threshold_comparison.
  if (_use_less_than_threshold_comparison && (nodal_val < threshold))
    return;

  if (!_use_less_than_threshold_comparison && (nodal_val > threshold))
    return;

  // Yay! A bubble -> Mark it!
  unsigned int map_num = _single_map_mode ? 0 : current_idx;
  if (live_region)
    _bubble_maps[map_num][node_id] = live_region;
  else
  {
    _bubble_maps[map_num][node_id] = ++_region_counts[map_num];
    _region_to_var_idx.push_back(current_idx);
  }

  std::vector< const Node * > neighbors;
  MeshTools::find_nodal_neighbors(_mesh.getMesh(), *node, _nodes_to_elem_map, neighbors);
  // Flood neighboring nodes that are also above this threshold with recursion
  for (unsigned int i = 0; i < neighbors.size(); ++i)
  {
    // Only recurse on nodes this processor can see
    if (_mesh.isSemiLocal(const_cast<Node *>(neighbors[i])))
      flood(neighbors[i], current_idx, _bubble_maps[map_num][node_id]);
  }
}

unsigned int
NodalFloodCount::appendPeriodicNeighborNodes(std::set<dof_id_type> & data) const
{
  unsigned int orig_size = data.size();

  /**
   * Now we will append our periodic neighbor information.  We treat the periodic neighbor nodes
   * much like we do ghosted nodes in a multi-processor setting.  If a bubble is sitting on a
   * periodic boundary we will simply add those periodic neighbors to the appropriate bubble
   * before packing up the data
   */
  std::set<dof_id_type> periodic_neighbors;

  // Using a typedef makes the code easier to understand and avoids repeating information.
  typedef std::multimap<dof_id_type, dof_id_type>::const_iterator IterType;

  for (std::set<dof_id_type>::iterator s_it = data.begin(); s_it != data.end(); ++s_it)
  {
    std::pair<IterType, IterType> iters = _periodic_node_map.equal_range(*s_it);

    for (IterType it = iters.first; it != iters.second; ++it)
      periodic_neighbors.insert(it->second);
  }

  // Now that we have all of the periodic_neighbors in our temporary set we need to add them to our input set
  data.insert(periodic_neighbors.begin(), periodic_neighbors.end());

  return data.size() - orig_size;
}

void
NodalFloodCount::updateRegionOffsets()
{
  if (_global_numbering)
    // Note: We never need to touch offset zero - it should *always* be zero
    for (unsigned int map_num=1; map_num < _maps_size; ++map_num)
      _region_offsets[map_num] = _region_offsets[map_num -1] + _region_counts[map_num - 1];
}

void
NodalFloodCount::calculateBubbleVolumes()
{
  Moose::perf_log.push("calculateBubbleVolume()", "NodalFloodCount");

  // Figure out which bubbles intersect the boundary if the user has enabled that capability.
  if (_compute_boundary_intersecting_volume)
  {
    // Create a std::set of node IDs which are on the boundary called all_boundary_node_ids.
    std::set<dof_id_type> all_boundary_node_ids;

    // Iterate over the boundary nodes, putting them into the std::set data structure
    MooseMesh::bnd_node_iterator
      boundary_nodes_it  = _mesh.bndNodesBegin(),
      boundary_nodes_end = _mesh.bndNodesEnd();
    for (; boundary_nodes_it != boundary_nodes_end; ++boundary_nodes_it)
    {
      BndNode * boundary_node = *boundary_nodes_it;
      all_boundary_node_ids.insert(boundary_node->_node->id());
    }

    // For each of the _maps_size BubbleData lists, determine if the set
    // of nodes includes any boundary nodes.
    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    {
      std::list<BubbleData>::iterator
        bubble_it = _bubble_sets[map_num].begin(),
        bubble_end = _bubble_sets[map_num].end();

      // Determine boundary intersection for each BubbleData object
      for (; bubble_it != bubble_end; ++bubble_it)
        bubble_it->_intersects_boundary = setsIntersect(all_boundary_node_ids.begin(), all_boundary_node_ids.end(),
                                                        bubble_it->_nodes.begin(), bubble_it->_nodes.end());
    }
  }

  // Size our temporary data structure
  std::vector<std::vector<Real> > bubble_volumes(_maps_size);
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    bubble_volumes[map_num].resize(_bubble_sets[map_num].size());

  // Clear pre-existing values and allocate space to store the volume
  // of the boundary-intersecting grains for each variable.
  _total_volume_intersecting_boundary.clear();
  _total_volume_intersecting_boundary.resize(_maps_size);

  // Loop over the active local elements.  For each variable, and for
  // each BubbleData object, check whether a majority of the element's
  // nodes belong to that Bubble, and if so assign the element's full
  // volume to that bubble.
  const MeshBase::const_element_iterator el_end = _mesh.getMesh().active_local_elements_end();
  for (MeshBase::const_element_iterator el = _mesh.getMesh().active_local_elements_begin(); el != el_end; ++el)
  {
    Elem *elem = *el;
    unsigned int elem_n_nodes = elem->n_nodes();
    Real curr_volume = elem->volume();

    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    {
      std::list<BubbleData>::const_iterator
        bubble_it = _bubble_sets[map_num].begin(),
        bubble_end = _bubble_sets[map_num].end();

      for (unsigned int bubble_counter = 0; bubble_it != bubble_end; ++bubble_it, ++bubble_counter)
      {
        // Count the number of nodes on this element which are flooded.
        unsigned int flooded_nodes = 0;
        for (unsigned int node = 0; node < elem_n_nodes; ++node)
        {
          dof_id_type node_id = elem->node(node);
          if (bubble_it->_nodes.find(node_id) != bubble_it->_nodes.end())
            ++flooded_nodes;
        }

        // If a majority of the nodes for this element are flooded,
        // assign its volume to the current bubble_counter entry.
        if (flooded_nodes >= elem_n_nodes / 2)
        {
          bubble_volumes[map_num][bubble_counter] += curr_volume;

          // If the current bubble also intersects the boundary, also
          // accumlate the volume into the total volume of bubbles
          // which intersect the boundary.
          if (bubble_it->_intersects_boundary)
            _total_volume_intersecting_boundary[map_num] += curr_volume;
        }
      }
    }
  }

  // If we're calculating boundary-intersecting volumes, we have to normalize it by the
  // volume of the entire domain.
  if (_compute_boundary_intersecting_volume)
  {
    // Compute the total area using a bounding box.  FIXME: this
    // assumes the domain is rectangular and 2D, and is probably a
    // little expensive so we should only do it once if possible.
    MeshTools::BoundingBox bbox = MeshTools::bounding_box(_mesh);
    Real total_volume = (bbox.max()(0)-bbox.min()(0))*(bbox.max()(1)-bbox.min()(1));

    // Sum up the partial boundary grain volume contributions from all processors
    _communicator.sum(_total_volume_intersecting_boundary);

    // Scale the boundary intersecting grain volumes by the total domain volume
    for (unsigned int i=0; i<_total_volume_intersecting_boundary.size(); ++i)
      _total_volume_intersecting_boundary[i] /= total_volume;
  }

  // Stick all the partial bubble volumes in one long single vector to be gathered on the root processor
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    _all_bubble_volumes.insert(_all_bubble_volumes.end(), bubble_volumes[map_num].begin(), bubble_volumes[map_num].end());

  // do all the sums!
  _communicator.sum(_all_bubble_volumes);

  std::sort(_all_bubble_volumes.begin(), _all_bubble_volumes.end(), std::greater<Real>());

  Moose::perf_log.pop("calculateBubbleVolume()", "NodalFloodCount");
}

template<>
unsigned long
NodalFloodCount::bytesHelper(std::list<BubbleData> container)
{
  unsigned long bytes = 0;
  for (std::list<BubbleData>::iterator it = container.begin(); it != container.end(); ++it)
    bytes += bytesHelper(it->_nodes) + sizeof(it->_var_idx);
  return bytes;
}

unsigned long
NodalFloodCount::calculateUsage() const
{
  unsigned long bytes = 0;

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    bytes += bytesHelper(_nodes_visited[map_num]);
    bytes += bytesHelper(_bubble_maps[map_num]);

    if (_var_index_mode)
      bytes += bytesHelper(_var_index_maps[map_num]);

    bytes += bytesHelper(_bubble_sets[map_num]);
  }

  bytes += sizeof(unsigned int) * _region_counts.size();
  bytes += sizeof(unsigned int) * _packed_data.size();
  bytes += sizeof(unsigned int) * _region_to_var_idx.size();
  bytes += sizeof(unsigned int) * _region_offsets.size();

  bytes += bytesHelper(_periodic_node_map);
  bytes += bytesHelper(_file_handles);

  bytes += sizeof(Real) * _all_bubble_volumes.size();

  // Not counted: _nodes_to_elem_map

  return bytes;
}

void
NodalFloodCount::formatBytesUsed() const
{
  std::stringstream oss;
  oss.precision(1);
  oss << std::fixed;
  if (_bytes_used >= 1<<30)
    oss << _name << " Memory Used: " << _bytes_used / Real(1<<30) << " GB\n";
  else if (_bytes_used >= 1<<20)
    oss << _name << " Memory Used: " << _bytes_used / Real(1<<20) << " MB\n";
  else if (_bytes_used >= 1<<10)
    oss << _name << " Memory Used: " << _bytes_used / Real(1<<10) << " KB\n";
  else
    oss << _name << " Memory Used: " << _bytes_used << " Bytes\n";
  _console << oss.str() << std::endl;
}


const std::vector<std::pair<unsigned int, unsigned int> > NodalFloodCount::_empty;
