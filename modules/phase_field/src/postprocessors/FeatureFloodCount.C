/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "FeatureFloodCount.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseUtils.h"
#include "IndirectSort.h"

#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "Assembly.h"

//libMesh includes
#include "libmesh/dof_map.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/point_locator_base.h"

#include <algorithm>
#include <limits>

template<>
void dataStore(std::ostream & stream, FeatureFloodCount::FeatureData & feature, void * context)
{
  /**
   * Not that _local_ids is not stored here. It's not needed for restart, and not needed
   * during the parallel merge operation
   */
  storeHelper(stream, feature._ghosted_ids, context);
  storeHelper(stream, feature._halo_ids, context);
  storeHelper(stream, feature._periodic_nodes, context);
  storeHelper(stream, feature._var_idx, context);
  storeHelper(stream, feature._bboxes, context);
  storeHelper(stream, feature._orig_ids, context);
  storeHelper(stream, feature._min_entity_id, context);
  storeHelper(stream, feature._volume, context);
  storeHelper(stream, feature._vol_count, context);
  storeHelper(stream, feature._centroid, context);
  storeHelper(stream, feature._status, context);
  storeHelper(stream, feature._intersects_boundary, context);
}

template<>
void dataStore(std::ostream & stream, MeshTools::BoundingBox & bbox, void * context)
{
  storeHelper(stream, bbox.min(), context);
  storeHelper(stream, bbox.max(), context);
}

template<>
void dataLoad(std::istream & stream, FeatureFloodCount::FeatureData & feature, void * context)
{
  /**
   * Not that _local_ids is not loaded here. It's not needed for restart, and not needed
   * during the parallel merge operation
   */
  loadHelper(stream, feature._ghosted_ids, context);
  loadHelper(stream, feature._halo_ids, context);
  loadHelper(stream, feature._periodic_nodes, context);
  loadHelper(stream, feature._var_idx, context);
  loadHelper(stream, feature._bboxes, context);
  loadHelper(stream, feature._orig_ids, context);
  loadHelper(stream, feature._min_entity_id, context);
  loadHelper(stream, feature._volume, context);
  loadHelper(stream, feature._vol_count, context);
  loadHelper(stream, feature._centroid, context);
  loadHelper(stream, feature._status, context);
  loadHelper(stream, feature._intersects_boundary, context);
}

template<>
void dataLoad(std::istream & stream, MeshTools::BoundingBox & bbox, void * context)
{
  loadHelper(stream, bbox.min(), context);
  loadHelper(stream, bbox.max(), context);
}

template<>
InputParameters validParams<FeatureFloodCount>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredCoupledVar("variable", "The variable(s) for which to find connected regions of interests, i.e. \"bubbles\".");
  params.addParam<Real>("threshold", 0.5, "The threshold value for which a new bubble may be started");
  params.addParam<Real>("connecting_threshold", "The threshold for which an existing bubble may be extended (defaults to \"threshold\")");
  params.addParam<Real>("volume_threshold", "The threshold used for calculating feature volumes (defaults to \"threshold\")");
  params.addParam<bool>("use_single_map", true, "Determine whether information is tracked per coupled variable or consolidated into one (default: true)");
  params.addParam<bool>("condense_map_info", false, "Determines whether we condense all the node values when in multimap mode (default: false)");
  params.addParam<bool>("use_global_numbering", true, "Determine whether or not global numbers are used to label bubbles on multiple maps (default: true)");
  params.addParam<bool>("enable_var_coloring", false, "Instruct the UO to populate the variable index map.");
  params.addParam<bool>("use_less_than_threshold_comparison", true, "Controls whether bubbles are defined to be less than or greater than the threshold value.");
  params.addParam<bool>("calculate_feature_volumes", false, "Flag to calculate feature volumes (Automatically set to True if \"bubble_volume_file\" is set)");
  params.addParam<FileName>("bubble_volume_file", "An optional file name where bubble volumes can be output.");
  params.addParam<bool>("compute_boundary_intersecting_volume", false, "If true, also compute the (normalized) volume of bubbles which intersect the boundary");
  params.set<bool>("use_displaced_mesh") = true;

  MooseEnum flood_type("NODAL ELEMENTAL", "ELEMENTAL");
  params.addParam<MooseEnum>("flood_entity_type", flood_type, "Determines whether the flood algorithm runs on nodes or elements");
  return params;
}


FeatureFloodCount::FeatureFloodCount(const InputParameters & parameters) :
    GeneralPostprocessor(parameters),
    Coupleable(this, false),
    MooseVariableDependencyInterface(),
    ZeroInterface(parameters),
    _vars(getCoupledMooseVars()),
    _threshold(getParam<Real>("threshold")),
    _connecting_threshold(isParamValid("connecting_threshold") ? getParam<Real>("connecting_threshold") : getParam<Real>("threshold")),
    _volume_threshold(isParamValid("volume_threshold") ? getParam<Real>("volume_threshold") : getParam<Real>("threshold")),
    _mesh(_subproblem.mesh()),
    _var_number(_vars[0]->number()),
    _single_map_mode(getParam<bool>("use_single_map")),
    _condense_map_info(getParam<bool>("condense_map_info")),
    _global_numbering(getParam<bool>("use_global_numbering")),
    _var_index_mode(getParam<bool>("enable_var_coloring")),
    _use_less_than_threshold_comparison(getParam<bool>("use_less_than_threshold_comparison")),
    _calculate_feature_volumes(getParam<bool>("calculate_feature_volumes") || isParamValid("bubble_volume_file")),
    _n_vars(_vars.size()),
    _maps_size(_single_map_mode ? 1 : _vars.size()),
    _n_procs(_app.n_processors()),
    _entities_visited(_vars.size()), // This map is always sized to the number of variables
    _feature_counts_per_map(_maps_size),
    _feature_count(0),
    _max_local_size(0),
    _partial_feature_sets(_maps_size),
    _feature_maps(_maps_size),
    _pbs(nullptr),
    _element_average_value(parameters.isParamValid("elem_avg_value") ? getPostprocessorValue("elem_avg_value") : _real_zero),
    _halo_ids(_maps_size),
    _compute_boundary_intersecting_volume(getParam<bool>("compute_boundary_intersecting_volume")),
    _is_elemental(getParam<MooseEnum>("flood_entity_type") == "ELEMENTAL" ? true : false)
{
  if (_var_index_mode)
    _var_index_maps.resize(_maps_size);
}

FeatureFloodCount::~FeatureFloodCount()
{
}

void
FeatureFloodCount::initialSetup()
{
  // Get a pointer to the PeriodicBoundaries buried in libMesh
  _pbs = _fe_problem.getNonlinearSystem().dofMap().get_periodic_boundaries();

  meshChanged();
}

void
FeatureFloodCount::initialize()
{
  // Clear the bubble marking maps and region counters and other data structures
  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
  {
    _feature_maps[map_num].clear();
    _partial_feature_sets[map_num].clear();

    if (_var_index_mode)
      _var_index_maps[map_num].clear();

    _halo_ids[map_num].clear();
  }

  _feature_sets.clear();

  // Calculate the thresholds for this iteration
  _step_threshold = _element_average_value + _threshold;
  _step_connecting_threshold = _element_average_value + _connecting_threshold;

  _all_feature_volumes.clear();
  _total_volume_intersecting_boundary.clear();

  _ghosted_entity_ids.clear();

  // Reset the feature count and max local size
  _feature_count = 0;
  _max_local_size = 0;

  clearDataStructures();
}

void
FeatureFloodCount::clearDataStructures()
{
  for (auto & map_ref : _entities_visited)
    map_ref.clear();
}

void
FeatureFloodCount::meshChanged()
{
  _mesh.buildPeriodicNodeMap(_periodic_node_map, _var_number, _pbs);

  // Build a new node to element map
  _nodes_to_elem_map.clear();
  MeshTools::build_nodes_to_elem_map(_mesh.getMesh(), _nodes_to_elem_map);

  /**
   * If the user has requested that we compute boundary intersecting volumes
   * then we need to build a set containing all of the necessary entities
   * to compare against. This will be elements for elemental flooding and nodes
   * for nodal flooding.
   */
  if (_compute_boundary_intersecting_volume)
  {
    _all_boundary_entity_ids.clear();
    if (_is_elemental)
      for (auto elem_it = _mesh.bndElemsBegin(), elem_end = _mesh.bndElemsEnd();
           elem_it != elem_end; ++elem_it)
        _all_boundary_entity_ids.insert((*elem_it)->_elem->id());
  }
}

void
FeatureFloodCount::execute()
{
  const auto end = _mesh.getMesh().active_local_elements_end();
  for (auto el = _mesh.getMesh().active_local_elements_begin(); el != end; ++el)
  {
    const Elem * current_elem = *el;

    // Loop over elements or nodes
    if (_is_elemental)
    {
      for (auto var_num = decltype(_n_vars)(0); var_num < _vars.size(); ++var_num)
        flood(current_elem, var_num, nullptr /* Designates inactive feature */);
    }
    else
    {
      auto n_nodes = current_elem->n_vertices();
      for (auto i = decltype(n_nodes)(0); i < n_nodes; ++i)
      {
        const Node * current_node = current_elem->get_node(i);

        for (auto var_num = decltype(_n_vars)(0); var_num < _vars.size(); ++var_num)
          flood(current_node, var_num, nullptr /* Designates inactive feature */);
      }
    }
  }
}

void FeatureFloodCount::communicateAndMerge()
{
  // First we need to transform the raw data into a usable data structure
  prepareDataForTransfer();

  /**
   * The libMesh packed range routines handle the communication of the individual
   * string buffers. Here we need to create a container to hold our type
   * to serialize. It'll always be size one because we are sending a single
   * byte stream of all the data to other processors. The stream need not be
   * the same size on all processors.
   */
  std::vector<std::string> send_buffers(1);

  /**
   * Additionally we need to create a different container to hold the received
   * byte buffers. The container type need not match the send container type.
   * However, We do know the number of incoming buffers (num processors) so we'll
   * go ahead and use a vector.
   */
  std::vector<std::string> recv_buffers;
  if (processor_id() == 0)
    recv_buffers.reserve(_app.n_processors());

  serialize(send_buffers[0]);

  // Free up as much memory as possible here before we do global communication
  clearDataStructures();

  /**
   * Send the data from all processors to the root to create a complete
   * global feature map.
   */
  _communicator.gather_packed_range(0, (void *)(nullptr), send_buffers.begin(), send_buffers.end(),
                                    std::back_inserter(recv_buffers));

  if (processor_id() == 0)
  {
    // The root process now needs to deserialize and merge all of the data
    deserialize(recv_buffers);
    recv_buffers.clear();

    mergeSets(true);

    /**
     * Perform a sort to give a parallel unique sorting to the identified features.
     * We use the "min_entity_id" inside each feature to assign it's position in the
     * sorted vector.
     */
    std::sort(_feature_sets.begin(), _feature_sets.end());

#ifndef NDEBUG
    /**
     * Sanity check. Now that we've sorted the flattened vector of features
     * we need to make sure that the counts vector still lines up appropriately
     * with each feature's _var_index.
     */
    unsigned int feature_offset = 0;
    for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
    {
      // Skip empty map checks
      if (_feature_counts_per_map[map_num] == 0)
        continue;

      // Check the begin and end of the current range
      auto range_front = feature_offset;
      auto range_back = feature_offset + _feature_counts_per_map[map_num] - 1;

      mooseAssert(range_front <= range_back && range_back < _feature_count, "Indexing error in feature sets");

      if (!_single_map_mode && (_feature_sets[range_front]._var_idx != map_num || _feature_sets[range_back]._var_idx != map_num))
        mooseError("Error in _feature_sets sorting, map index: " << map_num);

      feature_offset += _feature_counts_per_map[map_num];
    }
#endif
  }
}

void
FeatureFloodCount::buildLocalToGlobalIndices(std::vector<std::vector<unsigned int> > & local_to_global_all) const
{
  mooseAssert(processor_id() == 0, "This method must only be called on the root processor");

  local_to_global_all.resize(_n_procs, std::vector<unsigned int>(_max_local_size));

  for (decltype(_feature_sets.size()) i = 0, end_index = _feature_sets.size(); i < end_index; ++i)
  {
    // Get the local indices from the feature and build a map
    for (const auto & local_index_pair : _feature_sets[i]._orig_ids)
    {
      mooseAssert(local_index_pair.first < _n_procs, local_index_pair.first << ", " << _n_procs);
      mooseAssert(local_index_pair.second < _max_local_size, local_index_pair.second << ", " << _max_local_size);

                              // rank                 // local index
      local_to_global_all[local_index_pair.first][local_index_pair.second] = i;
    }
  }
}

void
FeatureFloodCount::scatterIndices(std::vector<std::vector<unsigned int> > & local_to_global_all)
{
  // First send _feature_count and _max_local_size for sizing vectors

  std::pair<unsigned int, unsigned int> sizes_pair(_feature_count, _max_local_size);
  _communicator.broadcast(sizes_pair);

  if (processor_id() == 0)
  {
    // TODO: Replace with MPI_SCATTER
    mooseAssert(local_to_global_all.size() == _n_procs, "local_to_global not sized properly");

    for (auto rank = decltype(_n_procs)(1); rank < _n_procs; ++rank)
      _communicator.send(rank, local_to_global_all[rank]);
  }
  else // Ranks 1..n
  {
    // Set _feature_count and _max_local_size on remaining ranks
    _feature_count = sizes_pair.first;
    _max_local_size = sizes_pair.second;

    // Now receive the local to global map from the root process
    _local_to_global_feature_map.resize(_max_local_size);
    _communicator.receive(0, _local_to_global_feature_map);
  }
}

void
FeatureFloodCount::finalize()
{
  communicateAndMerge();

  // local to global map (one per processor)
  std::vector<std::vector<unsigned int> > local_to_global_all;
  if (processor_id() == 0)
    buildLocalToGlobalIndices(local_to_global_all);

  scatterIndices(local_to_global_all);

  if (processor_id() != 0)
  {
    /**
     * The rest of the ranks just need to move their own data to the flat data structure.
     * Note that sorting is unecessary. The original ID inside of the unmerged feature
     * will contain the original position which can be used to look up the global id
     * number later after we receive the map from the root process.
     */
    _feature_sets.reserve(_max_local_size);
    for (auto & list_ref : _partial_feature_sets)
      for (auto & feature : list_ref)
        _feature_sets.emplace_back(std::move(feature));
  }

  // Populate _feature_maps and _var_index_maps
  updateFieldInfo();

  writeFeatureVolumeFile();
}

void
FeatureFloodCount::writeFeatureVolumeFile()
{
  if (_pars.isParamValid("bubble_volume_file") && processor_id() == 0)
  {
    // Pre-populated by updateFieldInfo
    mooseAssert(_all_feature_volumes.size() == _feature_count, "Incorrect number of volume entries");

    std::vector<Real> data;
    data.reserve(_all_feature_volumes.size() + _total_volume_intersecting_boundary.size() + 2);

    // Insert the current timestep and the simulation time into the data vector
    data.push_back(_fe_problem.timeStep());
    data.push_back(_fe_problem.time());

    // Insert the (sorted) bubble volumes into the data vector
    data.insert(data.end(), _all_feature_volumes.begin(), _all_feature_volumes.end());

    // If we are computing the boundary-intersecting volumes, insert
    // those numbers into the normalized boundary-intersecting bubble
    // volumes into the data vector.
    if (_compute_boundary_intersecting_volume)
      data.insert(data.end(), _total_volume_intersecting_boundary.begin(), _total_volume_intersecting_boundary.end());

    // Finally, write the file
    writeCSVFile(getParam<FileName>("bubble_volume_file"), data);
  }
}

Real
FeatureFloodCount::getValue()
{
  return _feature_count;
}

Real
FeatureFloodCount::getEntityValue(dof_id_type entity_id, FieldType field_type, unsigned int var_idx) const
{
  auto use_default = false;
  if (var_idx == std::numeric_limits<unsigned int>::max())
  {
    use_default = true;
    var_idx = 0;
  }

  mooseAssert(var_idx < _maps_size, "Index out of range");

  switch (field_type)
  {
    case FieldType::UNIQUE_REGION:
    {
      const auto entity_it = _feature_maps[var_idx].find(entity_id);

      if (entity_it != _feature_maps[var_idx].end())
        return entity_it->second; // + _region_offsets[var_idx];
      else
        return -1;
    }

    case FieldType::VARIABLE_COLORING:
    {
      const auto entity_it = _var_index_maps[var_idx].find(entity_id);

      if (entity_it != _var_index_maps[var_idx].end())
        return entity_it->second;
      else
        return -1;
    }

    case FieldType::GHOSTED_ENTITIES:
    {
      const auto entity_it = _ghosted_entity_ids.find(entity_id);

      if (entity_it != _ghosted_entity_ids.end())
        return entity_it->second;
      else
        return -1;
    }

    case FieldType::HALOS:
    {
      if (!use_default)
      {
        const auto entity_it = _halo_ids[var_idx].find(entity_id);
        if (entity_it != _halo_ids[var_idx].end())
          return entity_it->second;
      }
      else
      {
        // Showing halos in reverse order for backwards compatibility
        for (auto map_num = _maps_size; map_num-- /* don't compare greater than zero for unsigned */; )
        {
          const auto entity_it = _halo_ids[map_num].find(entity_id);

          if (entity_it != _halo_ids[map_num].end())
            return entity_it->second;
        }
      }
      return -1;
    }

    case FieldType::CENTROID:
    {
      if (_periodic_node_map.size())
        mooseDoOnce(mooseWarning("Centroids are not correct when using periodic boundaries, contact the MOOSE team"));

      // If this element contains the centroid of one of features, return it's index
      const auto * elem_ptr = _mesh.elemPtr(entity_id);

      for (const auto & feature : _feature_sets)
      {
        if (feature._status == Status::INACTIVE)
          continue;

        if (elem_ptr->contains_point(feature._centroid))
          return 1;
      }

      return 0;
    }

    default:
      return 0;
  }
}

const std::vector<std::pair<unsigned int, unsigned int> > &
FeatureFloodCount::getElementalValues(dof_id_type /*elem_id*/) const
{
  mooseDoOnce(mooseWarning("Method not implemented"));
  return _empty;
}

void
FeatureFloodCount::prepareDataForTransfer()
{
  MeshBase & mesh = _mesh.getMesh();

  for (auto & list_ref : _partial_feature_sets)
    for (auto & feature : list_ref)
    {
      for (auto & entity_id : feature._local_ids)
      {
        /**
         * Update the bounding box.
         *
         * Note: There will always be one and only one bbox while we are building up our
         * data structures because we haven't started to stitch together any regions yet.
         */
        if (_is_elemental)
          feature.updateBBoxExtremes(feature._bboxes[0], *mesh.elem(entity_id));
        else
          feature.updateBBoxExtremes(feature._bboxes[0], mesh.node(entity_id));

        // Save off the min entity id present in the feature to uniquely identify the feature regardless of n_procs
        feature._min_entity_id = std::min(feature._min_entity_id, entity_id);
      }

      // Now extend the bounding box by the halo region
      for (auto & halo_id : feature._halo_ids)
      {
        if (_is_elemental)
          feature.updateBBoxExtremes(feature._bboxes[0], *mesh.elem(halo_id));
        else
          feature.updateBBoxExtremes(feature._bboxes[0], mesh.node(halo_id));
      }

      // Periodic node ids
      appendPeriodicNeighborNodes(feature);
    }
}

void
FeatureFloodCount::serialize(std::string & serialized_buffer)
{
  // stream for serializing the _partial_feature_sets data structure to a byte stream
  std::ostringstream oss;

  /**
   * Call the MOOSE serialization routines to serialize this processor's data.
   * Note: The _partial_feature_sets data structure will be empty for all other processors
   */
  dataStore(oss, _partial_feature_sets, this);

  // Populate the passed in string pointer with the string stream's buffer contents
  serialized_buffer.assign(oss.str());
}

/**
 * This routine takes the vector of byte buffers (one for each processor), deserializes them
 * into a series of FeatureSet objects, and appends them to the _feature_sets data structure.
 *
 * Note: It is assumed that local processor information may already be stored in the _feature_sets
 * data structure so it is not cleared before insertion.
 */
void
FeatureFloodCount::deserialize(std::vector<std::string> & serialized_buffers)
{
  // The input string stream used for deserialization
  std::istringstream iss;

  mooseAssert(serialized_buffers.size() == _app.n_processors(), "Unexpected size of serialized_buffers: " << serialized_buffers.size());
  auto rank = processor_id();
  for (decltype(rank) proc_id = 0; proc_id < serialized_buffers.size(); ++proc_id)
  {
    /**
     * We should already have the local processor data in the features data structure.
     * Don't unpack the local buffer again.
     */
    if (proc_id == rank)
      continue;

    iss.str(serialized_buffers[proc_id]);    // populate the stream with a new buffer
    iss.clear();                          // reset the string stream state

    // Load the communicated data into all of the other processors' slots
    dataLoad(iss, _partial_feature_sets, this);
  }
}

void
FeatureFloodCount::mergeSets(bool use_periodic_boundary_info)
{
  Moose::perf_log.push("mergeSets()", "FeatureFloodCount");

  // Since we gathered only on the root process, we only need to merge sets on the root process.
  mooseAssert(processor_id() == 0, "mergeSets() should only be called on the root process");

  // Local variable used for sizing structures, it will be >= the actual number of features
  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
  {
    for (auto it1 = _partial_feature_sets[map_num].begin(); it1 != _partial_feature_sets[map_num].end(); /* No increment on it1 */)
    {
      bool merge_occured = false;
      for (auto it2 = _partial_feature_sets[map_num].begin(); it2 != _partial_feature_sets[map_num].end(); ++it2)
      {
        bool pb_intersect = false;
        if (it1 != it2 &&                                                    // Make sure that these iterators aren't pointing at the same set
            it1->_var_idx == it2->_var_idx &&                                // and that the sets have matching variable indices
             ((use_periodic_boundary_info &&                                 // and (if merging across periodic nodes
               (pb_intersect = it1->periodicBoundariesIntersect(*it2)))      //      do those periodic nodes intersect?
                 ||                                                          //      or
               (it1->boundingBoxesIntersect(*it2) &&                         //      if the region bboxes intersect
                it1->ghostedIntersect(*it2)                                  //      do the ghosted entities also intersect)
               )
             )
           )
        {
          it2->merge(std::move(*it1));

          // Insert the new entity at the end of the list so that it may be checked against all other partial features again
          _partial_feature_sets[map_num].emplace_back(std::move(*it2));

          /**
           * Now remove both halves the merged features: it2 contains the "moved" feature cell just inserted
           * at the back of the list, it1 contains the mostly empty other half. We have to be careful about the
           * order in which these two elements are deleted. We delete it2 first since we don't care where its
           * iterator points after the deletion. We are going to break out of this loop anyway. If we delete
           * it1 first, it may end up pointing at the same location as it2 which after the second deletion would
           * cause both of the iterators to be invalidated.
           */
          _partial_feature_sets[map_num].erase(it2);
          it1 = _partial_feature_sets[map_num].erase(it1); // it1 is incremented here!

          // A merge occurred, this is used to determine whether or not we increment the outer iterator
          merge_occured = true;

          // We need to start the list comparison over for the new it1 so break here
          break;
        }
      } // it2 loop

      if (!merge_occured) // No merges so we need to manually increment the outer iterator
        ++it1;

    } // it1 loop
  } // map loop

  /**
   * Now that the merges are complete we need to adjust the centroid, volume, and halos.
   * Additionally, To make several of the sorting and tracking algorithms more straightforward,
   * we will move the features into a flat vector. Finally we can count the final number
   * of features and find the max local index seen on any processor
   * Note: This is all occurring on rank 0 only!
   */

  // Offset where the current set of features with the same variable id starts in the flat vector
  unsigned int feature_offset = 0;
  // Max local size seen on any processor
  _max_local_size = 0;
  // Set the member feature count to zero and start counting the actual features
  _feature_count = 0;

  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
  {
    std::set<dof_id_type> set_difference;
    for (auto & feature : _partial_feature_sets[map_num])
    {
      // First we need to calculate the centroid now that we are doing merging all partial features
      if (feature._vol_count != 0)
        feature._centroid /= feature._vol_count;

      // Adjust the halo marking region
      set_difference.clear();
      std::set_difference(feature._halo_ids.begin(), feature._halo_ids.end(), feature._local_ids.begin(), feature._local_ids.end(),
                          std::insert_iterator<std::set<dof_id_type> >(set_difference, set_difference.begin()));
      feature._halo_ids.swap(set_difference);

      // Update the max ID
      for (const auto & local_index_pair : feature._orig_ids)
        _max_local_size = std::max(_max_local_size, local_index_pair.second);

      _feature_sets.emplace_back(std::move(feature));
      ++_feature_count;
    }

    // Record the feature numbers just for the current map
    _feature_counts_per_map[map_num] = _feature_count - feature_offset;

    // Now update the running feature count so we can calculate the next map's contribution
    feature_offset = _feature_count;

    // Clean up the "moved" objects
    _partial_feature_sets[map_num].clear();
  }

  // The local indices size is +1 from the max index
  _max_local_size += 1;

  // IMPORTANT: FeatureFloodCount::_feature_count and FeatureFloodCount::_max_local_size is set on rank 0 at this point

  Moose::perf_log.pop("mergeSets()", "FeatureFloodCount");
}

void
FeatureFloodCount::updateFieldInfo()
{
  // Whether or not we should store and write out volume information
  if (_calculate_feature_volumes && processor_id() == 0)
  {
    // store volumes per feature
    _all_feature_volumes.reserve(_feature_count);

    // store totals per variable (or smaller)
    _total_volume_intersecting_boundary.resize(_single_map_mode || _condense_map_info ? 1 : _maps_size);
  }

  for (decltype(_feature_sets.size()) i = 0, end_index = _feature_sets.size(); i < end_index; ++i)
  {
    auto & feature = _feature_sets[i];
    decltype(end_index) global_feature_number;

    if (processor_id() == 0)
      /**
       * If we are on processor zero, the global feature number is simply the current
       * index since we previously merged and sorted the partial features.
       */
      global_feature_number = i;
    else
    {
      /**
       * For the remaining ranks, obtaining the feature number requires us to
       * first obtain the original local index (stored inside of the feature).
       * Once we have that index, we can use it to local up the global id
       * in the local to global map.
       */
      mooseAssert(feature._orig_ids.size() == 1, "feature._orig_ids length doesn't make sense");
      auto local_id = feature._orig_ids.begin()->second;

      mooseAssert(local_id < _local_to_global_feature_map.size(), "local_id : " << local_id << " is out of range (" << _local_to_global_feature_map.size() << ')');
      global_feature_number = _local_to_global_feature_map[local_id];
    }

    // If the developer has requested _condense_map_info we'll make sure we only update the zeroth map
    auto map_idx = (_single_map_mode || _condense_map_info) ? decltype(feature._var_idx)(0) : feature._var_idx;

    // Loop over the entity ids of this feature and update our local map
    for (auto entity : feature._local_ids)
    {
      _feature_maps[map_idx][entity] = static_cast<int>(global_feature_number);

      if (_var_index_mode)
        _var_index_maps[map_idx][entity] = feature._var_idx;
    }

    // Loop over the halo ids to update cells with halo information
    for (auto entity : feature._halo_ids)
      _halo_ids[map_idx][entity] = static_cast<int>(global_feature_number);

    // Loop over the ghosted ids to update cells with ghost information
    for (auto entity : feature._ghosted_ids)
      _ghosted_entity_ids[entity] = 1;

    // Save off volume information
    if (_calculate_feature_volumes && processor_id() == 0)
    {
      _all_feature_volumes.push_back(feature._volume);
      if (feature._intersects_boundary)
        _total_volume_intersecting_boundary[map_idx] += feature._volume;
    }

    // TODO: Fixme
    if (!_global_numbering)
      mooseError("Local numbering currently disabled");

//    // If the user doesn't want a global numbering, we'll reset the feature_number for each map
//    if (!_global_numbering && feature._var_idx != old_var_index)
//      feature_number = 0;

//    old_var_index = feature._var_idx;
  }

  // Sort the feature volumes
  if (_calculate_feature_volumes && processor_id() == 0)
    std::sort(_all_feature_volumes.begin(), _all_feature_volumes.end(), std::greater<Real>());

//  mooseAssert(_feature_count == feature_number, "feature_number does not agree with previously calculated _feature_count");
}

void
FeatureFloodCount::flood(const DofObject * dof_object, unsigned long current_idx, FeatureData * feature)
{
  if (dof_object == nullptr)
    return;

  // Retrieve the id of the current entity
  auto entity_id = dof_object->id();

  // Has this entity already been marked? - if so move along
  if (_entities_visited[current_idx].find(entity_id) != _entities_visited[current_idx].end())
    return;

  // Determine which threshold to use based on whether this is an established region or
  // based on some derived class criteria.
  auto threshold = getThreshold(current_idx, feature != nullptr);

  // Get the value of the current variable for the current entity
  Real entity_value;
  const Elem * elem = nullptr;
  if (_is_elemental)
  {
    elem = static_cast<const Elem *>(dof_object);
    std::vector<Point> centroid(1, elem->centroid());
    _subproblem.reinitElemPhys(elem, centroid, 0);
    entity_value = _vars[current_idx]->sln()[0];
  }
  else
    entity_value = _vars[current_idx]->getNodalValue(*static_cast<const Node *>(dof_object));

  // This node hasn't been marked, is it in a feature?  We must respect
  // the user-selected value of _use_less_than_threshold_comparison.
  if (_use_less_than_threshold_comparison && (entity_value < threshold))
    return;

  if (!_use_less_than_threshold_comparison && (entity_value > threshold))
    return;

  /**
   * If we reach this point (i.e. we haven't returned early from this routine),
   * we've found a new mesh entity that's part of a feature. We need to mark
   * the entity as visited at this point (and not before!) to avoid infinite
   * recursion. If you mark the node to early you risk not coloring in a whole
   * feature any time a "connecting threshold" is used since we may have
   * already visited this entity earlier but it was in-between two thresholds.
   */
  _entities_visited[current_idx][entity_id] = true;

  auto map_num = _single_map_mode ? decltype(current_idx)(0) : current_idx;

  // New Feature (we need to create it and add it to our data structure)
  if (!feature)
    _partial_feature_sets[map_num].emplace_back(current_idx, _feature_count++, processor_id());

  // Get a handle to the feature we will update (always the last feature in the data structure)
  feature = &_partial_feature_sets[map_num].back();

  // Insert the current entity into the local ids map
  feature->_local_ids.insert(entity_id);

  /**
   * See if this particular entity cell contributes to the feature volume
   * 1) greater (or less) than the volume threshold which may be independent of the flooding threshold
   * 2) owned by this processor so it's not double counted
   */
  if (_is_elemental &&
      (entity_value >= _volume_threshold || (!_use_less_than_threshold_comparison && entity_value <= _volume_threshold)) &&
      processor_id() == dof_object->processor_id())
  {
    // TODO: Cache element volumes?
    feature->_volume += elem->volume();
    feature->_vol_count++;

    // Sum the centroid values for now, we'll average them later
    feature->_centroid += elem->centroid();

    // Does the volume intersect the boundary?
    if (_all_boundary_entity_ids.find(dof_object->id()) != _all_boundary_entity_ids.end())
      feature->_intersects_boundary = true;
  }

  if (_is_elemental)
    visitElementalNeighbors(static_cast<const Elem *>(dof_object), current_idx, feature, /*expand_halos_only =*/false);
  else
    visitNodalNeighbors(static_cast<const Node *>(dof_object), current_idx, feature, /*expand_halos_only =*/false);
}

Real
FeatureFloodCount::getThreshold(unsigned int /*current_idx*/, bool active_feature) const
{
  return active_feature ? _step_connecting_threshold : _step_threshold;
}

void
FeatureFloodCount::visitElementalNeighbors(const Elem * elem, unsigned long current_idx, FeatureData * feature, bool expand_halos_only)
{
  mooseAssert(elem, "Elem is NULL");

  std::vector<const Elem *> all_active_neighbors;

  // Loop over all neighbors (at the the same level as the current element)
  for (auto end = elem->n_neighbors(), i = 0u; i < end; ++i)
  {
    const Elem * neighbor_ancestor = elem->neighbor(i);
    if (neighbor_ancestor)
      // Retrieve only the active neighbors for each side of this element, append them to the list of active neighbors
      neighbor_ancestor->active_family_tree_by_neighbor(all_active_neighbors, elem, false);
  }

  visitNeighborsHelper(elem, all_active_neighbors, current_idx, feature, expand_halos_only);
}

void
FeatureFloodCount::visitNodalNeighbors(const Node * node, unsigned long current_idx, FeatureData * feature, bool expand_halos_only)
{
  mooseAssert(node, "Node is NULL");

  std::vector<const Node *> all_active_neighbors;
  MeshTools::find_nodal_neighbors(_mesh.getMesh(), *node, _nodes_to_elem_map, all_active_neighbors);

  visitNeighborsHelper(node, all_active_neighbors, current_idx, feature, expand_halos_only);
}

template<typename T>
void
FeatureFloodCount::visitNeighborsHelper(const T * curr_entity, std::vector<const T *> neighbor_entities, unsigned long current_idx,
                                        FeatureData * feature, bool expand_halos_only)
{
  // Loop over all active element neighbors
  for (const auto neighbor : neighbor_entities)
  {
    if (neighbor)
    {
      if (expand_halos_only)
        feature->_halo_ids.insert(neighbor->id());

      else
      {
        auto my_processor_id = processor_id();

        if (neighbor->processor_id() != my_processor_id)
          feature->_ghosted_ids.insert(curr_entity->id());

        /**
         * Only recurse where we own this entity. We might step outside of the
         * ghosted region if we recurse where we don't own the current entity.
         */
        if (curr_entity->processor_id() == my_processor_id)
        {
          /**
           * Premark neighboring entities with a halo mark. These
           * entities may or may not end up being part of the feature.
           * We will not update the _entities_visited data structure
           * here.
           */
          feature->_halo_ids.insert(neighbor->id());

          flood(neighbor, current_idx, feature);
        }
      }
    }
  }
}

void
FeatureFloodCount::appendPeriodicNeighborNodes(FeatureData & data) const
{
  if (_is_elemental)
  {
    for (auto entity : data._local_ids)
    {
      Elem * elem = _mesh.elemPtr(entity);

      for (auto end = elem->n_nodes(), node_n = 0u; node_n < end; ++node_n)
      {
        auto iters = _periodic_node_map.equal_range(elem->node(node_n));

        for (auto it = iters.first; it != iters.second; ++it)
        {
          data._periodic_nodes.insert(it->first);
          data._periodic_nodes.insert(it->second);
        }
      }
    }
  }
  else
  {
    for (auto entity : data._local_ids)
    {
      auto iters = _periodic_node_map.equal_range(entity);

      for (auto it = iters.first; it != iters.second; ++it)
      {
        data._periodic_nodes.insert(it->first);
        data._periodic_nodes.insert(it->second);
      }
    }
  }
}

void
FeatureFloodCount::FeatureData::updateBBoxExtremes(MeshTools::BoundingBox & bbox, const Point & node)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    bbox.min()(i) = std::min(bbox.min()(i), node(i));
    bbox.max()(i) = std::max(bbox.max()(i), node(i));
  }
}

void
FeatureFloodCount::FeatureData::updateBBoxExtremes(MeshTools::BoundingBox & bbox, const Elem & elem)
{
  for (auto n = elem.n_nodes(), node_n = 0u; node_n < n; ++node_n)
    updateBBoxExtremes(bbox, *(elem.get_node(node_n)));
}

void
FeatureFloodCount::FeatureData::updateBBoxExtremes(MeshTools::BoundingBox & bbox, const MeshTools::BoundingBox & rhs_bbox)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    bbox.min()(i) = std::min(bbox.min()(i), rhs_bbox.min()(i));
    bbox.max()(i) = std::max(bbox.max()(i), rhs_bbox.max()(i));
  }
}


bool
FeatureFloodCount::FeatureData::boundingBoxesIntersect(const FeatureData & rhs) const
{
  // See if any of the bounding boxes in either FeatureData object intersect
  for (const auto & bbox_lhs : _bboxes)
    for (const auto & bbox_rhs : rhs._bboxes)
      if (bbox_lhs.intersect(bbox_rhs))
        return true;

  return false;
}


bool
FeatureFloodCount::FeatureData::halosIntersect(const FeatureData & rhs) const
{
  return setsIntersect(_halo_ids.begin(), _halo_ids.end(),
                       rhs._halo_ids.begin(), rhs._halo_ids.end());
}

bool
FeatureFloodCount::FeatureData::periodicBoundariesIntersect(const FeatureData & rhs) const
{
  return setsIntersect(_periodic_nodes.begin(), _periodic_nodes.end(),
                       rhs._periodic_nodes.begin(), rhs._periodic_nodes.end());
}

bool
FeatureFloodCount::FeatureData::ghostedIntersect(const FeatureData & rhs) const
{
  return setsIntersect(_ghosted_ids.begin(), _ghosted_ids.end(),
                       rhs._ghosted_ids.begin(), rhs._ghosted_ids.end());
}

void
FeatureFloodCount::FeatureData::merge(FeatureData && rhs)
{
  std::set<dof_id_type> set_union;

  /**
   * Even though we've determined that these two partial regions need to be merged, we don't necessarily know if the _ghost_ids intersect.
   * We could be in this branch because the periodic boundaries intersect but that doesn't tell us anything about whether or not the ghost_region
   * also intersects. If the _ghost_ids intersect, that means that we are merging along a periodic boundary, not across one. In this case the
   * bounding box(s) need to be expanded.
   */
  std::set_union(_periodic_nodes.begin(), _periodic_nodes.end(), rhs._periodic_nodes.begin(), rhs._periodic_nodes.end(),
                 std::insert_iterator<std::set<dof_id_type> >(set_union, set_union.begin()));
  _periodic_nodes.swap(set_union);

  set_union.clear();
  std::set_union(_local_ids.begin(), _local_ids.end(), rhs._local_ids.begin(), rhs._local_ids.end(),
                 std::insert_iterator<std::set<dof_id_type> >(set_union, set_union.begin()));
  _local_ids.swap(set_union);

  set_union.clear();
  std::set_union(_halo_ids.begin(), _halo_ids.end(), rhs._halo_ids.begin(), rhs._halo_ids.end(),
                 std::insert_iterator<std::set<dof_id_type> >(set_union, set_union.begin()));
  _halo_ids.swap(set_union);

  set_union.clear();
  std::set_union(_ghosted_ids.begin(), _ghosted_ids.end(), rhs._ghosted_ids.begin(), rhs._ghosted_ids.end(),
                 std::insert_iterator<std::set<dof_id_type> >(set_union, set_union.begin()));
  // Was there overlap in the physical region?
  bool physical_intersection = (_ghosted_ids.size() + rhs._ghosted_ids.size() > set_union.size());
  _ghosted_ids.swap(set_union);

  /**
   * If we had a physical intersection, we need to expand boxes. If we had a virtual (periodic) intersection we need to preserve
   * all of the boxes from each of the regions' sets.
   */
  if (physical_intersection)
    expandBBox(rhs);
  else
    std::move(rhs._bboxes.begin(), rhs._bboxes.end(), std::back_inserter(_bboxes));

  // Keep track of the original ids so we can notify other processors of the local to global mapping
  _orig_ids.splice(_orig_ids.end(), std::move(rhs._orig_ids));

  // Update the min feature id
  _min_entity_id = std::min(_min_entity_id, rhs._min_entity_id);

  _volume += rhs._volume;
  _vol_count += rhs._vol_count;
  _centroid += rhs._centroid;
}

void
FeatureFloodCount::FeatureData::expandBBox(const FeatureData & rhs)
{
  std::vector<bool> intersected_boxes(rhs._bboxes.size(), false);

  auto box_expanded = false;
  for (auto & bbox : _bboxes)
    for (size_t j = 0; j < rhs._bboxes.size(); ++j)
      if (bbox.intersect(rhs._bboxes[j]))
      {
        updateBBoxExtremes(bbox, rhs._bboxes[j]);
        intersected_boxes[j] = true;
        box_expanded = true;
      }

  // Any bounding box in the rhs vector that doesn't intersect
  // needs to be appended to the lhs vector
  for (size_t j = 0; j < intersected_boxes.size(); ++j)
    if (!intersected_boxes[j])
      _bboxes.push_back(rhs._bboxes[j]);

  // Error check
  if (!box_expanded)
  {
    std::ostringstream oss;
    oss << "LHS BBoxes:\n";
    for (unsigned int i = 0; i < _bboxes.size(); ++i)
      oss << "Max: " << _bboxes[i].max() << " Min: " << _bboxes[i].min() << '\n';

    oss << "RHS BBoxes:\n";
    for (unsigned int i = 0; i < rhs._bboxes.size(); ++i)
      oss << "Max: " << rhs._bboxes[i].max() << " Min: " << rhs._bboxes[i].min() << '\n';

    mooseError("No Bounding Boxes Expanded - This is a catastrophic error!\n" << oss.str());
  }
}

std::ostream &
operator<<(std::ostream & out, const FeatureFloodCount::FeatureData & feature)
{
  static const bool debug = true;

  if (debug)
  {
    out << "Ghosted Entities: ";
    for (auto ghosted_id : feature._ghosted_ids)
      out << ghosted_id << " ";

    out << "\nLocal Entities: ";
    for (auto local_id : feature._local_ids)
      out << local_id << " ";

    out << "\nHalo Entities: ";
    for (auto halo_id : feature._halo_ids)
      out << halo_id << " ";

    out << "\nPeriodic Node IDs: ";
    for (auto periodic_node : feature._periodic_nodes)
      out << periodic_node << " ";
  }

  out << "\nBBoxes:";
  Real volume = 0;
  for (const auto & bbox : feature._bboxes)
  {
    out << "\nMax: " << bbox.max() << " Min: " << bbox.min();
    volume += (bbox.max()(0) - bbox.min()(0)) * (bbox.max()(1) - bbox.min()(1)) *
      (MooseUtils::absoluteFuzzyEqual(bbox.max()(2), bbox.min()(2)) ? 1 : bbox.max()(2) - bbox.min()(2));
  }

  out << "\nStatus: ";
  if (feature._status == FeatureFloodCount::Status::CLEAR)
    out << "CLEAR";
  if (static_cast<bool>(feature._status & FeatureFloodCount::Status::MARKED))
    out << " MARKED";
  if (static_cast<bool>(feature._status & FeatureFloodCount::Status::DIRTY))
    out << " DIRTY";
  if (static_cast<bool>(feature._status & FeatureFloodCount::Status::INACTIVE))
    out << " INACTIVE";

  if (debug)
  {
    out << "\nOrig IDs (rank, index): ";
    for (const auto & orig_pair : feature._orig_ids)
      out << '(' << orig_pair.first << ", " << orig_pair.second << ") ";
    out << "\nVolume: " << volume;
    out << "\nVar_idx: " << feature._var_idx;
    out << "\nMin Entity ID: " << feature._min_entity_id;
  }
  out << "\n\n";

  return out;
}

const std::vector<std::pair<unsigned int, unsigned int> > FeatureFloodCount::_empty;
