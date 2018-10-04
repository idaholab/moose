//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FeatureFloodCount.h"
#include "IndirectSort.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "MooseVariable.h"
#include "SubProblem.h"

#include "Assembly.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

#include "libmesh/dof_map.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/remote_elem.h"

#include <algorithm>
#include <limits>

template <>
void
dataStore(std::ostream & stream, FeatureFloodCount::FeatureData & feature, void * context)
{
  /**
   * Note that _local_ids is not stored here. It's not needed for restart, and not needed
   * during the parallel merge operation
   */
  storeHelper(stream, feature._ghosted_ids, context);
  storeHelper(stream, feature._halo_ids, context);
  storeHelper(stream, feature._disjoint_halo_ids, context);
  storeHelper(stream, feature._periodic_nodes, context);
  storeHelper(stream, feature._var_index, context);
  storeHelper(stream, feature._id, context);
  storeHelper(stream, feature._bboxes, context);
  storeHelper(stream, feature._orig_ids, context);
  storeHelper(stream, feature._min_entity_id, context);
  storeHelper(stream, feature._vol_count, context);
  storeHelper(stream, feature._centroid, context);
  storeHelper(stream, feature._status, context);
  storeHelper(stream, feature._intersects_boundary, context);
}

template <>
void
dataStore(std::ostream & stream, MeshTools::BoundingBox & bbox, void * context)
{
  storeHelper(stream, bbox.min(), context);
  storeHelper(stream, bbox.max(), context);
}

template <>
void
dataLoad(std::istream & stream, FeatureFloodCount::FeatureData & feature, void * context)
{
  /**
   * Note that _local_ids is not loaded here. It's not needed for restart, and not needed
   * during the parallel merge operation
   */
  loadHelper(stream, feature._ghosted_ids, context);
  loadHelper(stream, feature._halo_ids, context);
  loadHelper(stream, feature._disjoint_halo_ids, context);
  loadHelper(stream, feature._periodic_nodes, context);
  loadHelper(stream, feature._var_index, context);
  loadHelper(stream, feature._id, context);
  loadHelper(stream, feature._bboxes, context);
  loadHelper(stream, feature._orig_ids, context);
  loadHelper(stream, feature._min_entity_id, context);
  loadHelper(stream, feature._vol_count, context);
  loadHelper(stream, feature._centroid, context);
  loadHelper(stream, feature._status, context);
  loadHelper(stream, feature._intersects_boundary, context);
}

template <>
void
dataLoad(std::istream & stream, MeshTools::BoundingBox & bbox, void * context)
{
  loadHelper(stream, bbox.min(), context);
  loadHelper(stream, bbox.max(), context);
}

// Utility routines
void updateBBoxExtremesHelper(MeshTools::BoundingBox & bbox, const Point & node);
void updateBBoxExtremesHelper(MeshTools::BoundingBox & bbox, const Elem & elem);
bool areElemListsMergeable(const std::list<dof_id_type> & elem_list1,
                           const std::list<dof_id_type> & elem_list2,
                           MeshBase & mesh);

registerMooseObject("PhaseFieldApp", FeatureFloodCount);

template <>
InputParameters
validParams<FeatureFloodCount>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params += validParams<BoundaryRestrictable>();

  params.addRequiredCoupledVar(
      "variable",
      "The variable(s) for which to find connected regions of interests, i.e. \"features\".");
  params.addParam<Real>(
      "threshold", 0.5, "The threshold value for which a new feature may be started");
  params.addParam<Real>(
      "connecting_threshold",
      "The threshold for which an existing feature may be extended (defaults to \"threshold\")");
  params.addParam<bool>("use_single_map",
                        true,
                        "Determine whether information is tracked per "
                        "coupled variable or consolidated into one "
                        "(default: true)");
  params.addParam<bool>(
      "condense_map_info",
      false,
      "Determines whether we condense all the node values when in multimap mode (default: false)");
  params.addParam<bool>("use_global_numbering",
                        true,
                        "Determine whether or not global numbers are "
                        "used to label features on multiple maps "
                        "(default: true)");
  params.addParam<bool>("enable_var_coloring",
                        false,
                        "Instruct the Postprocessor to populate the variable index map.");
  params.addParam<bool>(
      "compute_halo_maps",
      false,
      "Instruct the Postprocessor to communicate proper halo information to all ranks");
  params.addParam<bool>("compute_var_to_feature_map",
                        false,
                        "Instruct the Postprocessor to compute the active vars to features map");
  params.addParam<bool>(
      "use_less_than_threshold_comparison",
      true,
      "Controls whether features are defined to be less than or greater than the threshold value.");

  /**
   * The FeatureFloodCount and derived objects should not to operate on the displaced mesh. These
   * objects consume variable values from the nonlinear system and use a lot of raw geometric
   * element information from the mesh. If you use the displaced system with EBSD information for
   * instance, you'll have difficulties reconciling the difference between the coordinates from the
   * EBSD data file and the potential displacements applied via boundary conditions.
   */
  params.set<bool>("use_displaced_mesh") = false;

  params.addParamNamesToGroup("use_single_map condense_map_info use_global_numbering", "Advanced");

  MooseEnum flood_type("NODAL ELEMENTAL", "ELEMENTAL");
  params.addParam<MooseEnum>("flood_entity_type",
                             flood_type,
                             "Determines whether the flood algorithm runs on nodes or elements");

  params.addClassDescription("The object is able to find and count \"connected components\" in any "
                             "solution field or number of solution fields. A primary example would "
                             "be to count \"bubbles\".");
  return params;
}

FeatureFloodCount::FeatureFloodCount(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    Coupleable(this, false),
    MooseVariableDependencyInterface(),
    BoundaryRestrictable(this, false),
    _fe_vars(getCoupledMooseVars()),
    _vars(getCoupledStandardMooseVars()),
    _dof_map(_vars[0]->dofMap()),
    _threshold(getParam<Real>("threshold")),
    _connecting_threshold(isParamValid("connecting_threshold")
                              ? getParam<Real>("connecting_threshold")
                              : getParam<Real>("threshold")),
    _mesh(_subproblem.mesh()),
    _var_number(_fe_vars[0]->number()),
    _single_map_mode(getParam<bool>("use_single_map")),
    _condense_map_info(getParam<bool>("condense_map_info")),
    _global_numbering(getParam<bool>("use_global_numbering")),
    _var_index_mode(getParam<bool>("enable_var_coloring")),
    _compute_halo_maps(getParam<bool>("compute_halo_maps")),
    _compute_var_to_feature_map(getParam<bool>("compute_var_to_feature_map")),
    _use_less_than_threshold_comparison(getParam<bool>("use_less_than_threshold_comparison")),
    _n_vars(_fe_vars.size()),
    _maps_size(_single_map_mode ? 1 : _fe_vars.size()),
    _n_procs(_app.n_processors()),
    _feature_counts_per_map(_maps_size),
    _feature_count(0),
    _partial_feature_sets(_maps_size),
    _feature_maps(_maps_size),
    _pbs(nullptr),
    _element_average_value(parameters.isParamValid("elem_avg_value")
                               ? getPostprocessorValue("elem_avg_value")
                               : _real_zero),
    _halo_ids(_maps_size),
    _is_elemental(getParam<MooseEnum>("flood_entity_type") == "ELEMENTAL"),
    _is_master(processor_id() == 0),
    _distribute_merge_work(_app.n_processors() >= _maps_size && _maps_size > 1),
    _execute_timer(registerTimedSection("execute", 1)),
    _merge_timer(registerTimedSection("mergeFeatures", 2)),
    _finalize_timer(registerTimedSection("finalize", 1)),
    _comm_and_merge(registerTimedSection("communicateAndMerge", 2)),
    _expand_halos(registerTimedSection("expandEdgeHalos", 2)),
    _update_field_info(registerTimedSection("updateFieldInfo", 2)),
    _prepare_for_transfer(registerTimedSection("prepareDataForTransfer", 2)),
    _consolidate_merged_features(registerTimedSection("consolidateMergedFeatures", 2))
{
  if (_var_index_mode)
    _var_index_maps.resize(_maps_size);

  addMooseVariableDependency(_fe_vars);

  _is_boundary_restricted = boundaryRestricted();
}

FeatureFloodCount::~FeatureFloodCount() {}

void
FeatureFloodCount::initialSetup()
{
  // We need one map per coupled variable for normal runs to support overlapping features
  _entities_visited.resize(_vars.size());

  // Get a pointer to the PeriodicBoundaries buried in libMesh
  _pbs = _fe_problem.getNonlinearSystemBase().dofMap().get_periodic_boundaries();

  meshChanged();

  /**
   * Size the empty var to features vector to the number of coupled variables.
   * This empty vector (but properly sized) vector is returned for elements
   * that are queried but are not in the structure (which also shouldn't happen).
   * The user is warned in this case but this helps avoid extra bounds checking
   * in user code and avoids segfaults.
   */
  _empty_var_to_features.resize(_n_vars, invalid_id);
}

void
FeatureFloodCount::initialize()
{
  // Clear the feature marking maps and region counters and other data structures
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

  _ghosted_entity_ids.clear();

  // Reset the feature count and max local size
  _feature_count = 0;

  _entity_var_to_features.clear();

  for (auto & map_ref : _entities_visited)
    map_ref.clear();
}

void
FeatureFloodCount::clearDataStructures()
{
}

void
FeatureFloodCount::meshChanged()
{
  _point_locator = _mesh.getMesh().sub_point_locator();

  _mesh.buildPeriodicNodeMap(_periodic_node_map, _var_number, _pbs);

  // Build a new node to element map
  _nodes_to_elem_map.clear();
  MeshTools::build_nodes_to_elem_map(_mesh.getMesh(), _nodes_to_elem_map);

  /**
   * We need to build a set containing all of the boundary entities
   * to compare against. This will be elements for elemental flooding.
   * Volumes for nodal flooding is not supported
   */
  _all_boundary_entity_ids.clear();
  if (_is_elemental)
    for (auto elem_it = _mesh.bndElemsBegin(), elem_end = _mesh.bndElemsEnd(); elem_it != elem_end;
         ++elem_it)
      _all_boundary_entity_ids.insert((*elem_it)->_elem->id());
}

void
FeatureFloodCount::execute()
{
  TIME_SECTION(_execute_timer);

  // Iterate only over boundaries if restricted
  if (_is_boundary_restricted)
  {
    mooseInfo("Using EXPERIMENTAL boundary restricted FeatureFloodCount object!\n");

    // Set the boundary range pointer for use during flooding
    _bnd_elem_range = _mesh.getBoundaryElementRange();

    auto rank = processor_id();

    for (const auto & belem : *_bnd_elem_range)
    {
      const Elem * elem = belem->_elem;
      BoundaryID boundary_id = belem->_bnd_id;

      if (elem->processor_id() == rank)
      {
        if (hasBoundary(boundary_id))
          for (auto var_num = beginIndex(_vars); var_num < _vars.size(); ++var_num)
            flood(elem, var_num);
      }
    }
  }
  else // Normal volumetric operation
  {
    for (const auto & current_elem : _mesh.getMesh().active_local_element_ptr_range())
    {
      // Loop over elements or nodes
      if (_is_elemental)
      {
        for (auto var_num = beginIndex(_vars); var_num < _vars.size(); ++var_num)
          flood(current_elem, var_num);
      }
      else
      {
        auto n_nodes = current_elem->n_vertices();
        for (auto i = decltype(n_nodes)(0); i < n_nodes; ++i)
        {
          const Node * current_node = current_elem->get_node(i);

          for (auto var_num = beginIndex(_vars); var_num < _vars.size(); ++var_num)
            flood(current_node, var_num);
        }
      }
    }
  }
}

void
FeatureFloodCount::communicateAndMerge()
{
  TIME_SECTION(_comm_and_merge);

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
  std::vector<std::string> recv_buffers, deserialize_buffers;

  /**
   * When we distribute merge work, we are reducing computational work by adding more communication.
   * Each of the first _n_vars processors will receive one variable worth of information to merge.
   * After each of those processors has merged that information, it'll be sent to the master
   * processor where final consolidation will occur.
   */
  if (_distribute_merge_work)
  {
    auto rank = processor_id();
    bool is_merging_processor = rank < _n_vars;

    if (is_merging_processor)
      recv_buffers.reserve(_app.n_processors());

    for (auto i = decltype(_n_vars)(0); i < _n_vars; ++i)
    {
      serialize(send_buffers[0], i);

      /**
       * Send the data from all processors to the first _n_vars processors to create a complete
       * global feature maps for each variable.
       */
      _communicator.gather_packed_range(i,
                                        (void *)(nullptr),
                                        send_buffers.begin(),
                                        send_buffers.end(),
                                        std::back_inserter(recv_buffers));

      /**
       * A call to gather_packed_range seems to populate the receiving buffer on all processors, not
       * just the receiving buffer on the actual receiving processor. If we plan to call this
       * function repeatedly, we must clear the buffers each time on all non-receiving processors.
       * On the actual receiving processor, we'll save off the buffer for use later.
       */
      if (rank == i)
        recv_buffers.swap(deserialize_buffers);
      else
        recv_buffers.clear();
    }

    // Setup a new communicator for doing merging communication operations
    Parallel::Communicator merge_comm;

    // TODO: Update to MPI_UNDEFINED when libMesh bug is fixed.
    _communicator.split(is_merging_processor ? 0 : 1, rank, merge_comm);

    if (is_merging_processor)
    {
      /**
       * The FeatureFloodCount and derived algorithms rely on having the data structures intact on
       * all non-zero ranks. This is because local-only information (local entities) is never
       * communicated and thus must remain intact. However, the distributed merging will destroy
       * that information. The easiest thing to do is to swap out the data structure while
       * we perform the distributed merge work.
       */
      std::vector<std::list<FeatureData>> tmp_data(_partial_feature_sets.size());
      tmp_data.swap(_partial_feature_sets);

      deserialize(deserialize_buffers, processor_id());

      send_buffers[0].clear();
      recv_buffers.clear();
      deserialize_buffers.clear();

      // Merge one variable's worth of data
      mergeSets();

      // Now we need to serialize again to send to the master (only the processors who did work)
      serialize(send_buffers[0]);

      // Free up as much memory as possible here before we do global communication
      clearDataStructures();

      /**
       * Send the data from the merging processors to the root to create a complete global feature
       * map.
       */
      merge_comm.gather_packed_range(0,
                                     (void *)(nullptr),
                                     send_buffers.begin(),
                                     send_buffers.end(),
                                     std::back_inserter(recv_buffers));

      if (_is_master)
      {
        // The root process now needs to deserialize all of the data
        deserialize(recv_buffers);

        send_buffers[0].clear();
        recv_buffers.clear();

        consolidateMergedFeatures(&tmp_data);
      }
      else
        // Restore our original data on non-zero ranks
        tmp_data.swap(_partial_feature_sets);
    }
  }

  // Serialized merging (master does all the work)
  else
  {
    if (_is_master)
      recv_buffers.reserve(_app.n_processors());

    serialize(send_buffers[0]);

    // Free up as much memory as possible here before we do global communication
    clearDataStructures();

    /**
     * Send the data from all processors to the root to create a complete
     * global feature map.
     */
    _communicator.gather_packed_range(0,
                                      (void *)(nullptr),
                                      send_buffers.begin(),
                                      send_buffers.end(),
                                      std::back_inserter(recv_buffers));

    if (_is_master)
    {
      // The root process now needs to deserialize all of the data
      deserialize(recv_buffers);
      recv_buffers.clear();

      mergeSets();

      consolidateMergedFeatures();
    }
  }

  // Make sure that feature count is communicated to all ranks
  _communicator.broadcast(_feature_count);
}

void
FeatureFloodCount::sortAndLabel()
{
  mooseAssert(_is_master, "sortAndLabel can only be called on the master");

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
  for (auto map_num = beginIndex(_feature_counts_per_map); map_num < _maps_size; ++map_num)
  {
    // Skip empty map checks
    if (_feature_counts_per_map[map_num] == 0)
      continue;

    // Check the begin and end of the current range
    auto range_front = feature_offset;
    auto range_back = feature_offset + _feature_counts_per_map[map_num] - 1;

    mooseAssert(range_front <= range_back && range_back < _feature_count,
                "Indexing error in feature sets");

    if (!_single_map_mode && (_feature_sets[range_front]._var_index != map_num ||
                              _feature_sets[range_back]._var_index != map_num))
      mooseError("Error in _feature_sets sorting, map index: ", map_num);

    feature_offset += _feature_counts_per_map[map_num];
  }
#endif

  // Label the features with an ID based on the sorting (processor number independent value)
  for (auto i = beginIndex(_feature_sets); i < _feature_sets.size(); ++i)
    if (_feature_sets[i]._id == invalid_id)
      _feature_sets[i]._id = i;
}

void
FeatureFloodCount::buildLocalToGlobalIndices(std::vector<std::size_t> & local_to_global_all,
                                             std::vector<int> & counts) const
{
  mooseAssert(_is_master, "This method must only be called on the root processor");

  counts.assign(_n_procs, 0);
  // Now size the individual counts vectors based on the largest index seen per processor
  for (const auto & feature : _feature_sets)
    for (const auto & local_index_pair : feature._orig_ids)
    {
      // local_index_pair.first = ranks, local_index_pair.second = local_index
      mooseAssert(local_index_pair.first < _n_procs, "Processor ID is out of range");
      if (local_index_pair.second >= static_cast<std::size_t>(counts[local_index_pair.first]))
        counts[local_index_pair.first] = local_index_pair.second + 1;
    }

  // Build the offsets vector
  unsigned int globalsize = 0;
  std::vector<int> offsets(_n_procs); // Type is signed for use with the MPI API
  for (auto i = beginIndex(offsets); i < offsets.size(); ++i)
  {
    offsets[i] = globalsize;
    globalsize += counts[i];
  }

  // Finally populate the master vector
  local_to_global_all.resize(globalsize, FeatureFloodCount::invalid_size_t);
  for (const auto & feature : _feature_sets)
  {
    // Get the local indices from the feature and build a map
    for (const auto & local_index_pair : feature._orig_ids)
    {
      auto rank = local_index_pair.first;
      mooseAssert(rank < _n_procs, rank << ", " << _n_procs);

      auto local_index = local_index_pair.second;
      auto stacked_local_index = offsets[rank] + local_index;

      mooseAssert(stacked_local_index < globalsize,
                  "Global index: " << stacked_local_index << " is out of range");
      local_to_global_all[stacked_local_index] = feature._id;
    }
  }
}

void
FeatureFloodCount::buildFeatureIdToLocalIndices(unsigned int max_id)
{
  _feature_id_to_local_index.assign(max_id + 1, invalid_size_t);
  for (auto feature_index = beginIndex(_feature_sets); feature_index < _feature_sets.size();
       ++feature_index)
  {
    if (_feature_sets[feature_index]._status != Status::INACTIVE)
    {
      mooseAssert(_feature_sets[feature_index]._id <= max_id,
                  "Feature ID out of range(" << _feature_sets[feature_index]._id << ')');
      _feature_id_to_local_index[_feature_sets[feature_index]._id] = feature_index;
    }
  }
}

void
FeatureFloodCount::finalize()
{
  TIME_SECTION(_finalize_timer);

  // Gather all information on processor zero and merge
  communicateAndMerge();

  // Sort and label the features
  if (_is_master)
    sortAndLabel();

  // Send out the local to global mappings
  scatterAndUpdateRanks();

  // Populate _feature_maps and _var_index_maps
  updateFieldInfo();
}

const std::vector<unsigned int> &
FeatureFloodCount::getVarToFeatureVector(dof_id_type elem_id) const
{
  mooseDoOnce(if (!_compute_var_to_feature_map) mooseError(
      "Please set \"compute_var_to_feature_map = true\" to use this interface method"));

  const auto pos = _entity_var_to_features.find(elem_id);
  if (pos != _entity_var_to_features.end())
  {
    mooseAssert(pos->second.size() == _n_vars, "Variable to feature vector not sized properly");
    return pos->second;
  }
  else
    return _empty_var_to_features;
}

void
FeatureFloodCount::scatterAndUpdateRanks()
{
  // local to global map (one per processor)
  std::vector<int> counts;
  std::vector<std::size_t> local_to_global_all;
  if (_is_master)
    buildLocalToGlobalIndices(local_to_global_all, counts);

  // Scatter local_to_global indices to all processors and store in class member variable
  _communicator.scatter(local_to_global_all, counts, _local_to_global_feature_map);

  std::size_t largest_global_index = std::numeric_limits<std::size_t>::lowest();
  if (!_is_master)
  {
    _feature_sets.resize(_local_to_global_feature_map.size());

    /**
     * On non-root processors we can't maintain the full _feature_sets data structure since
     * we don't have all of the global information. We'll move the items from the partial
     * feature sets into a flat structure maintaining order and update the internal IDs
     * with the proper global ID.
     */
    for (auto & list_ref : _partial_feature_sets)
    {
      for (auto & feature : list_ref)
      {
        mooseAssert(feature._orig_ids.size() == 1, "feature._orig_ids length doesn't make sense");

        auto global_index = FeatureFloodCount::invalid_size_t;
        auto local_index = feature._orig_ids.begin()->second;

        if (local_index < _local_to_global_feature_map.size())
          global_index = _local_to_global_feature_map[local_index];

        if (global_index != FeatureFloodCount::invalid_size_t)
        {
          if (global_index > largest_global_index)
            largest_global_index = global_index;

          // Set the correct global index
          feature._id = global_index;

          /**
           * Important: Make sure we clear the local status if we received a valid global
           * index for this feature. It's possible that we have a status of INVALID
           * on the local processor because there was never any starting threshold found.
           * However, the root processor wouldn't have sent an index if it didn't find
           * a starting threshold connected to our local piece.
           */
          feature._status &= ~Status::INACTIVE;

          // Move the feature into the correct place
          _feature_sets[local_index] = std::move(feature);
        }
      }
    }
  }
  else
  {
    for (auto global_index : local_to_global_all)
      if (global_index != FeatureFloodCount::invalid_size_t && global_index > largest_global_index)
        largest_global_index = global_index;
  }

  buildFeatureIdToLocalIndices(largest_global_index);
}

Real
FeatureFloodCount::getValue()
{
  return static_cast<Real>(_feature_count);
}

std::size_t
FeatureFloodCount::getNumberActiveFeatures() const
{
  // Note: This value is parallel consistent, see FeatureFloodCount::communicateAndMerge()
  return _feature_count;
}

std::size_t
FeatureFloodCount::getTotalFeatureCount() const
{
  /**
   * Since the FeatureFloodCount object doesn't maintain any information about
   * features between invocations. The maximum id in use is simply the number of
   * features.
   */
  return _feature_count;
}

unsigned int
FeatureFloodCount::getFeatureVar(unsigned int feature_id) const
{
  // Some processors don't contain the largest feature id, in that case we just return invalid_id
  if (feature_id >= _feature_id_to_local_index.size())
    return invalid_id;

  auto local_index = _feature_id_to_local_index[feature_id];
  if (local_index != invalid_size_t)
  {
    mooseAssert(local_index < _feature_sets.size(), "local_index out of bounds");
    return _feature_sets[local_index]._status != Status::INACTIVE
               ? _feature_sets[local_index]._var_index
               : invalid_id;
  }

  return invalid_id;
}

bool
FeatureFloodCount::doesFeatureIntersectBoundary(unsigned int feature_id) const
{
  // TODO: This information is not parallel consistent when using FeatureFloodCounter

  // Some processors don't contain the largest feature id, in that case we just return invalid_id
  if (feature_id >= _feature_id_to_local_index.size())
    return false;

  auto local_index = _feature_id_to_local_index[feature_id];

  if (local_index != invalid_size_t)
  {
    mooseAssert(local_index < _feature_sets.size(), "local_index out of bounds");
    return _feature_sets[local_index]._status != Status::INACTIVE
               ? _feature_sets[local_index]._intersects_boundary
               : invalid_id;
  }

  return false;
}

Point
FeatureFloodCount::featureCentroid(unsigned int feature_id) const
{
  if (feature_id >= _feature_id_to_local_index.size())
    return invalid_id;

  auto local_index = _feature_id_to_local_index[feature_id];

  Real invalid_coord = std::numeric_limits<Real>::max();
  Point p(invalid_coord, invalid_coord, invalid_coord);
  if (local_index != invalid_size_t)
  {
    mooseAssert(local_index < _feature_sets.size(), "local_index out of bounds");
    p = _feature_sets[local_index]._centroid;
  }
  return p;
}

Real
FeatureFloodCount::getEntityValue(dof_id_type entity_id,
                                  FieldType field_type,
                                  std::size_t var_index) const
{
  auto use_default = false;
  if (var_index == invalid_size_t)
  {
    use_default = true;
    var_index = 0;
  }

  mooseAssert(var_index < _maps_size, "Index out of range");

  switch (field_type)
  {
    case FieldType::UNIQUE_REGION:
    {
      const auto entity_it = _feature_maps[var_index].find(entity_id);

      if (entity_it != _feature_maps[var_index].end())
        return entity_it->second; // + _region_offsets[var_index];
      else
        return -1;
    }

    case FieldType::VARIABLE_COLORING:
    {
      mooseAssert(
          _var_index_mode,
          "\"enable_var_coloring\" must be set to true to pull back the VARIABLE_COLORING field");

      const auto entity_it = _var_index_maps[var_index].find(entity_id);

      if (entity_it != _var_index_maps[var_index].end())
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
        const auto entity_it = _halo_ids[var_index].find(entity_id);
        if (entity_it != _halo_ids[var_index].end())
          return entity_it->second;
      }
      else
      {
        // Showing halos in reverse order for backwards compatibility
        for (auto map_num = _maps_size;
             map_num-- /* don't compare greater than zero for unsigned */;)
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
        mooseDoOnce(mooseWarning(
            "Centroids are not correct when using periodic boundaries, contact the MOOSE team"));

      // If this element contains the centroid of one of features, return one
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

void
FeatureFloodCount::prepareDataForTransfer()
{
  TIME_SECTION(_prepare_for_transfer);

  MeshBase & mesh = _mesh.getMesh();

  FeatureData::container_type local_ids_no_ghost, set_difference;

  for (auto & list_ref : _partial_feature_sets)
  {
    for (auto & feature : list_ref)
    {
      // Periodic node ids
      appendPeriodicNeighborNodes(feature);

      /**
       * If using a vector container, we need to sort all of the data structures for later
       * operations such as checking for intersection and merging. The following "sort" function
       * does nothing when invoked on a std::set.
       */
      FeatureFloodCount::sort(feature._ghosted_ids);
      FeatureFloodCount::sort(feature._local_ids);
      FeatureFloodCount::sort(feature._halo_ids);
      FeatureFloodCount::sort(feature._disjoint_halo_ids);
      FeatureFloodCount::sort(feature._periodic_nodes);

      // Now extend the bounding box by the halo region
      if (_is_elemental)
        feature.updateBBoxExtremes(mesh);
      else
      {
        for (auto & halo_id : feature._halo_ids)
          updateBBoxExtremesHelper(feature._bboxes[0], mesh.node(halo_id));
      }

      mooseAssert(!feature._local_ids.empty(), "local entity ids cannot be empty");

      /**
       * Save off the min entity id present in the feature to uniquely
       * identify the feature regardless of n_procs
       */
      feature._min_entity_id = *feature._local_ids.begin();
    }
  }
}

void
FeatureFloodCount::serialize(std::string & serialized_buffer, unsigned int var_num)
{
  // stream for serializing the _partial_feature_sets data structure to a byte stream
  std::ostringstream oss;

  mooseAssert(var_num == invalid_id || var_num < _partial_feature_sets.size(),
              "var_num out of range");

  // Serialize everything
  if (var_num == invalid_id)
    dataStore(oss, _partial_feature_sets, this);
  else
    dataStore(oss, _partial_feature_sets[var_num], this);

  // Populate the passed in string pointer with the string stream's buffer contents
  serialized_buffer.assign(oss.str());
}

void
FeatureFloodCount::deserialize(std::vector<std::string> & serialized_buffers, unsigned int var_num)
{
  // The input string stream used for deserialization
  std::istringstream iss;

  auto rank = processor_id();

  for (auto proc_id = beginIndex(serialized_buffers); proc_id < serialized_buffers.size();
       ++proc_id)
  {
    /**
     * Usually we have the local processor data already in the _partial_feature_sets data structure.
     * However, if we are doing distributed merge work, we also need to preserve all of the original
     * data for use in later stages of the algorithm so it'll have been swapped out with clean
     * buffers. This leaves us a choice, either we just duplicate the Features from the original
     * data structure after we've swapped out the buffer, or we go ahead and unpack data that we
     * would normally already have. So during distributed merging, that's exactly what we'll do.
     * Later however when the master is doing the final consolidating, we'll opt to just skip
     * the local unpacking. To tell the difference, between these two modes, we just need to
     * see if a var_num was passed in.
     */
    if (var_num == invalid_id && proc_id == rank)
      continue;

    iss.str(serialized_buffers[proc_id]); // populate the stream with a new buffer
    iss.clear();                          // reset the string stream state

    // Load the gathered data into the data structure.
    if (var_num == invalid_id)
      dataLoad(iss, _partial_feature_sets, this);
    else
      dataLoad(iss, _partial_feature_sets[var_num], this);
  }
}

void
FeatureFloodCount::mergeSets()
{
  TIME_SECTION(_merge_timer);

  // When working with _distribute_merge_work all of the maps will be empty except for one
  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
  {
    for (auto it1 = _partial_feature_sets[map_num].begin();
         it1 != _partial_feature_sets[map_num].end();
         /* No increment on it1 */)
    {
      bool merge_occured = false;
      for (auto it2 = _partial_feature_sets[map_num].begin();
           it2 != _partial_feature_sets[map_num].end();
           ++it2)
      {
        if (it1 != it2 && areFeaturesMergeable(*it1, *it2))
        {
          it2->merge(std::move(*it1));

          /**
           * Insert the new entity at the end of the list so that it may be checked against all
           * other partial features again.
           */
          _partial_feature_sets[map_num].emplace_back(std::move(*it2));

          /**
           * Now remove both halves the merged features: it2 contains the "moved" feature cell just
           * inserted at the back of the list, it1 contains the mostly empty other half. We have to
           * be careful about the order in which these two elements are deleted. We delete it2 first
           * since we don't care where its iterator points after the deletion. We are going to break
           * out of this loop anyway. If we delete it1 first, it may end up pointing at the same
           * location as it2 which after the second deletion would cause both of the iterators to be
           * invalidated.
           */
          _partial_feature_sets[map_num].erase(it2);
          it1 = _partial_feature_sets[map_num].erase(it1); // it1 is incremented here!

          // A merge occurred, this is used to determine whether or not we increment the outer
          // iterator
          merge_occured = true;

          // We need to start the list comparison over for the new it1 so break here
          break;
        }
      } // it2 loop

      if (!merge_occured) // No merges so we need to manually increment the outer iterator
        ++it1;

    } // it1 loop
  }   // map loop
}

void
FeatureFloodCount::consolidateMergedFeatures(std::vector<std::list<FeatureData>> * saved_data)
{
  TIME_SECTION(_consolidate_merged_features);

  /**
   * Now that the merges are complete we need to adjust the centroid, and halos.
   * Additionally, To make several of the sorting and tracking algorithms more straightforward,
   * we will move the features into a flat vector. Finally we can count the final number of
   * features and find the max local index seen on any processor
   * Note: This is all occurring on rank 0 only!
   */
  mooseAssert(_is_master, "cosolidateMergedFeatures() may only be called on the master processor");

  // Offset where the current set of features with the same variable id starts in the flat vector
  unsigned int feature_offset = 0;
  // Set the member feature count to zero and start counting the actual features
  _feature_count = 0;

  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
  {
    for (auto & feature : _partial_feature_sets[map_num])
    {
      if (saved_data)
      {
        for (auto it = (*saved_data)[map_num].begin(); it != (*saved_data)[map_num].end();
             /* no increment */)
        {
          if (feature.canConsolidate(*it))
          {
            feature.consolidate(std::move(*it));
            it = (*saved_data)[map_num].erase(it); // increment
          }
          else
            ++it;
        }
      }

      // If after merging we still have an inactive feature, discard it
      if (feature._status == Status::CLEAR)
      {
        // First we need to calculate the centroid now that we are doing merging all partial
        // features
        if (feature._vol_count != 0)
          feature._centroid /= feature._vol_count;

        _feature_sets.emplace_back(std::move(feature));
        ++_feature_count;
      }
    }

    // Record the feature numbers just for the current map
    _feature_counts_per_map[map_num] = _feature_count - feature_offset;

    // Now update the running feature count so we can calculate the next map's contribution
    feature_offset = _feature_count;

    // Clean up the "moved" objects
    _partial_feature_sets[map_num].clear();
  }

  /**
   * IMPORTANT: FeatureFloodCount::_feature_count is set on rank 0 at this point but
   * we can't broadcast it here because this routine is not collective.
   */
}

bool
FeatureFloodCount::areFeaturesMergeable(const FeatureData & f1, const FeatureData & f2) const
{
  return f1.mergeable(f2);
}

void
FeatureFloodCount::updateFieldInfo()
{
  for (auto i = beginIndex(_feature_sets); i < _feature_sets.size(); ++i)
  {
    auto & feature = _feature_sets[i];

    // If the developer has requested _condense_map_info we'll make sure we only update the zeroth
    // map
    auto map_index = (_single_map_mode || _condense_map_info) ? decltype(feature._var_index)(0)
                                                              : feature._var_index;

    // Loop over the entity ids of this feature and update our local map
    for (auto entity : feature._local_ids)
    {
      _feature_maps[map_index][entity] = static_cast<int>(feature._id);

      if (_var_index_mode)
        _var_index_maps[map_index][entity] = feature._var_index;

      // Fill in the data structure that keeps track of all features per elem
      if (_compute_var_to_feature_map)
      {
        auto insert_pair = moose_try_emplace(
            _entity_var_to_features, entity, std::vector<unsigned int>(_n_vars, invalid_id));
        auto & vec_ref = insert_pair.first->second;
        vec_ref[feature._var_index] = feature._id;
      }
    }

    if (_compute_halo_maps)
      // Loop over the halo ids to update cells with halo information
      for (auto entity : feature._halo_ids)
        _halo_ids[map_index][entity] = static_cast<int>(feature._id);

    // Loop over the ghosted ids to update cells with ghost information
    for (auto entity : feature._ghosted_ids)
      _ghosted_entity_ids[entity] = 1;

    // TODO: Fixme
    if (!_global_numbering)
      mooseError("Local numbering currently disabled");
  }
}

bool
FeatureFloodCount::flood(const DofObject * dof_object, std::size_t current_index)

{
  //  if (dof_object == nullptr || dof_object == libMesh::remote_elem)
  //    return false;
  mooseAssert(dof_object, "DOF object is nullptr");
  mooseAssert(_entity_queue.empty(), "Entity queue is not empty when starting a feature");

  // Kick off the exploration of a new feature
  _entity_queue.push_front(dof_object);

  bool return_value = false;
  FeatureData * feature = nullptr;
  while (!_entity_queue.empty())
  {
    const DofObject * curr_dof_object = _entity_queue.back();
    _entity_queue.pop_back();

    // Retrieve the id of the current entity
    auto entity_id = curr_dof_object->id();

    // Has this entity already been marked? - if so move along
    if (current_index != invalid_size_t &&
        _entities_visited[current_index].find(entity_id) != _entities_visited[current_index].end())
      continue;

    // Are we outside of the range we should be working in?
    if (_is_elemental)
    {
      const Elem & elem = static_cast<const Elem &>(*curr_dof_object);

      if (!_dof_map.is_evaluable(elem))
        continue;
    }

    // See if the current entity either starts a new feature or continues an existing feature
    auto new_id = invalid_id; // Writable reference to hold an optional id;
    Status status =
        Status::INACTIVE; // Status is inactive until we find an entity above the starting threshold

    if (!isNewFeatureOrConnectedRegion(curr_dof_object, current_index, feature, status, new_id))
    {
      // If we have an active feature, we just found a halo entity
      if (feature)
        feature->_halo_ids.insert(feature->_halo_ids.end(), entity_id);
      continue;
    }

    mooseAssert(current_index != invalid_size_t, "current_index is invalid");

    /**
     * If we reach this point (i.e. we haven't continued to the next queue entry),
     * we've found a new mesh entity that's part of a feature. We need to mark
     * the entity as visited at this point (and not before!) to avoid infinite
     * recursion. If you mark the node too early you risk not coloring in a whole
     * feature any time a "connecting threshold" is used since we may have
     * already visited this entity earlier but it was in-between two thresholds.
     */
    return_value = true;
    _entities_visited[current_index].insert(entity_id);

    auto map_num = _single_map_mode ? decltype(current_index)(0) : current_index;

    // New Feature (we need to create it and add it to our data structure)
    if (!feature)
    {
      _partial_feature_sets[map_num].emplace_back(
          current_index, _feature_count++, processor_id(), status);

      // Get a handle to the feature we will update (always the last feature in the data structure)
      feature = &_partial_feature_sets[map_num].back();

      // If new_id is valid, we'll set it in the feature here.
      if (new_id != invalid_id)
        feature->_id = new_id;
    }

    // Insert the current entity into the local ids data structure
    feature->_local_ids.insert(feature->_local_ids.end(), entity_id);

    /**
     * See if this particular entity cell contributes to the centroid calculation. We
     * only deal with elemental floods and only count it if it's owned by the current
     * processor to avoid skewing the result.
     */
    if (_is_elemental && processor_id() == curr_dof_object->processor_id())
    {
      const Elem * elem = static_cast<const Elem *>(curr_dof_object);

      // Keep track of how many elements participate in the centroid averaging
      feature->_vol_count++;

      // Sum the centroid values for now, we'll average them later
      feature->_centroid += elem->centroid();

      // Does the volume intersect the boundary?
      if (_all_boundary_entity_ids.find(elem->id()) != _all_boundary_entity_ids.end())
        feature->_intersects_boundary = true;
    }

    if (_is_elemental)
      visitElementalNeighbors(static_cast<const Elem *>(curr_dof_object),
                              feature,
                              /*expand_halos_only =*/false,
                              /*disjoint_only =*/false);
    else
      visitNodalNeighbors(static_cast<const Node *>(curr_dof_object),
                          feature,
                          /*expand_halos_only =*/false);
  }

  return return_value;
}

Real FeatureFloodCount::getThreshold(std::size_t /*current_index*/) const
{
  return _step_threshold;
}

Real FeatureFloodCount::getConnectingThreshold(std::size_t /*current_index*/) const
{
  return _step_connecting_threshold;
}

bool
FeatureFloodCount::compareValueWithThreshold(Real entity_value, Real threshold) const
{
  return ((_use_less_than_threshold_comparison && (entity_value >= threshold)) ||
          (!_use_less_than_threshold_comparison && (entity_value <= threshold)));
}

bool
FeatureFloodCount::isNewFeatureOrConnectedRegion(const DofObject * dof_object,
                                                 std::size_t & current_index,
                                                 FeatureData *& feature,
                                                 Status & status,
                                                 unsigned int & /*new_id*/)
{
  // Get the value of the current variable for the current entity
  Real entity_value;
  if (_is_elemental)
  {
    const Elem * elem = static_cast<const Elem *>(dof_object);
    std::vector<Point> centroid(1, elem->centroid());
    _subproblem.reinitElemPhys(elem, centroid, 0);
    entity_value = _vars[current_index]->sln()[0];
  }
  else
    entity_value = _vars[current_index]->getNodalValue(*static_cast<const Node *>(dof_object));

  // If the value compares against our starting threshold, this is definitely part of a feature
  // we'll keep
  if (compareValueWithThreshold(entity_value, getThreshold(current_index)))
  {
    Status * status_ptr = &status;

    if (feature)
      status_ptr = &feature->_status;

    // Update an existing feature's status or clear the flag on the passed in status
    *status_ptr &= ~Status::INACTIVE;
    return true;
  }

  /**
   * If the value is _only_ above the connecting threshold, it's still part of a feature but
   * possibly part of one that we'll discard if there is never any starting threshold encountered.
   */
  return compareValueWithThreshold(entity_value, getConnectingThreshold(current_index));
}

void
FeatureFloodCount::expandPointHalos()
{
  const auto & node_to_elem_map = _mesh.nodeToActiveSemilocalElemMap();
  FeatureData::container_type expanded_local_ids;
  auto my_processor_id = processor_id();

  /**
   * To expand the feature element region to the actual flooded region (nodal basis)
   * we need to add in all point neighbors of the current local region for each feature.
   * This is because the elemental variable influence spreads from the elemental data out
   * exactly one element from every mesh point.
   */
  for (auto & list_ref : _partial_feature_sets)
  {
    for (auto & feature : list_ref)
    {
      expanded_local_ids.clear();

      for (auto entity : feature._local_ids)
      {
        const Elem * elem = _mesh.elemPtr(entity);
        mooseAssert(elem, "elem pointer is NULL");

        // Get the nodes on a current element so that we can add in point neighbors
        auto n_nodes = elem->n_vertices();
        for (auto i = decltype(n_nodes)(0); i < n_nodes; ++i)
        {
          const Node * current_node = elem->get_node(i);

          auto elem_vector_it = node_to_elem_map.find(current_node->id());
          if (elem_vector_it == node_to_elem_map.end())
            mooseError("Error in node to elem map");

          const auto & elem_vector = elem_vector_it->second;

          std::copy(elem_vector.begin(),
                    elem_vector.end(),
                    std::insert_iterator<FeatureData::container_type>(expanded_local_ids,
                                                                      expanded_local_ids.end()));

          // Now see which elements need to go into the ghosted set
          for (auto entity : elem_vector)
          {
            const Elem * neighbor = _mesh.elemPtr(entity);
            mooseAssert(neighbor, "neighbor pointer is NULL");

            if (neighbor->processor_id() != my_processor_id)
              feature._ghosted_ids.insert(feature._ghosted_ids.end(), elem->id());
          }
        }
      }

      // Replace the existing local ids with the expanded local ids
      feature._local_ids.swap(expanded_local_ids);

      // Copy the expanded local_ids into the halo_ids container
      feature._halo_ids = feature._local_ids;
    }
  }
}

void
FeatureFloodCount::expandEdgeHalos(unsigned int num_layers_to_expand)
{
  if (num_layers_to_expand == 0)
    return;

  TIME_SECTION(_expand_halos);

  for (auto & list_ref : _partial_feature_sets)
  {
    for (auto & feature : list_ref)
    {
      for (auto halo_level = decltype(num_layers_to_expand)(0); halo_level < num_layers_to_expand;
           ++halo_level)
      {
        /**
         * Create a copy of the halo set so that as we insert new ids into the
         * set we don't continue to iterate on those new ids.
         */
        FeatureData::container_type orig_halo_ids(feature._halo_ids);
        for (auto entity : orig_halo_ids)
        {
          if (_is_elemental)
            visitElementalNeighbors(_mesh.elemPtr(entity),
                                    &feature,
                                    /*expand_halos_only =*/true,
                                    /*disjoint_only =*/false);
          else
            visitNodalNeighbors(_mesh.nodePtr(entity),
                                &feature,
                                /*expand_halos_only =*/true);
        }

        /**
         * We have to handle disjoint halo IDs slightly differently. Once you are disjoint, you
         * can't go back so make sure that we keep placing these IDs in the disjoint set.
         */
        FeatureData::container_type disjoint_orig_halo_ids(feature._disjoint_halo_ids);
        for (auto entity : disjoint_orig_halo_ids)
        {
          if (_is_elemental)
            visitElementalNeighbors(_mesh.elemPtr(entity),

                                    &feature,
                                    /*expand_halos_only =*/true,
                                    /*disjoint_only =*/true);
          else
            visitNodalNeighbors(_mesh.nodePtr(entity),

                                &feature,
                                /*expand_halos_only =*/true);
        }
      }
    }
  }
}

void
FeatureFloodCount::visitElementalNeighbors(const Elem * elem,
                                           FeatureData * feature,
                                           bool expand_halos_only,
                                           bool disjoint_only)
{
  mooseAssert(elem, "Elem is NULL");

  std::vector<const Elem *> all_active_neighbors;
  MeshBase & mesh = _mesh.getMesh();

  // Loop over all neighbors (at the the same level as the current element)
  for (auto i = decltype(elem->n_neighbors())(0); i < elem->n_neighbors(); ++i)
  {
    const Elem * neighbor_ancestor = nullptr;
    bool topological_neighbor = false;

    /**
     * Retrieve only the active neighbors for each side of this element, append them to the list
     * of active neighbors
     */
    neighbor_ancestor = elem->neighbor(i);
    if (neighbor_ancestor)
    {
      if (neighbor_ancestor == libMesh::remote_elem)
        continue;

      neighbor_ancestor->active_family_tree_by_neighbor(all_active_neighbors, elem, false);
    }
    else
    {
      neighbor_ancestor = elem->topological_neighbor(i, mesh, *_point_locator, _pbs);

      /**
       * If the current element (passed into this method) doesn't have a connected neighbor but
       * does have a topological neighbor, this might be a new disjoint region that we'll
       * need to represent with a separate bounding box. To find out for sure, we'll need
       * see if the new neighbors are present in any of the halo or disjoint halo sets. If
       * they are not present, this is a new region.
       */
      if (neighbor_ancestor)
      {
        neighbor_ancestor->active_family_tree_by_topological_neighbor(
            all_active_neighbors, elem, mesh, *_point_locator, _pbs, false);

        topological_neighbor = true;
      }
      else
      {
        /**
         * This neighbor is NULL which means we need to expand the bounding box here in case this
         * grain is up against multiple domain edges so we don't end up with a degenerate bounding
         * box.
         */
        updateBBoxExtremesHelper(feature->_bboxes[0], *elem);
      }
    }

    visitNeighborsHelper(elem,
                         all_active_neighbors,
                         feature,
                         expand_halos_only,
                         topological_neighbor,
                         disjoint_only);

    all_active_neighbors.clear();
  }
}

void
FeatureFloodCount::visitNodalNeighbors(const Node * node,
                                       FeatureData * feature,
                                       bool expand_halos_only)
{
  mooseAssert(node, "Node is NULL");

  std::vector<const Node *> all_active_neighbors;
  MeshTools::find_nodal_neighbors(_mesh.getMesh(), *node, _nodes_to_elem_map, all_active_neighbors);

  visitNeighborsHelper(node, all_active_neighbors, feature, expand_halos_only, false, false);
}

template <typename T>
void
FeatureFloodCount::visitNeighborsHelper(const T * curr_entity,
                                        std::vector<const T *> neighbor_entities,
                                        FeatureData * feature,
                                        bool expand_halos_only,
                                        bool topological_neighbor,
                                        bool disjoint_only)
{
  // Loop over all active element neighbors
  for (const auto neighbor : neighbor_entities)
  {
    if (neighbor && (!_is_boundary_restricted || isBoundaryEntity(neighbor)))
    {
      if (expand_halos_only)
      {
        auto entity_id = neighbor->id();

        if (topological_neighbor || disjoint_only)
          feature->_disjoint_halo_ids.insert(feature->_disjoint_halo_ids.end(), entity_id);
        else if (feature->_local_ids.find(entity_id) == feature->_local_ids.end())
          feature->_halo_ids.insert(feature->_halo_ids.end(), entity_id);
      }
      else
      {
        auto my_processor_id = processor_id();

        if (!topological_neighbor && neighbor->processor_id() != my_processor_id)
          feature->_ghosted_ids.insert(feature->_ghosted_ids.end(), curr_entity->id());

        /**
         * Only recurse where we own this entity and it's a topologically connected entity. We
         * shouldn't even attempt to flood to the periodic boundary because we won't have solution
         * information and if we are using DistributedMesh we probably won't have geometric
         * information either.
         *
         * When we only recurse on entities we own, we can never get more than one away from
         * a local entity which should be in the ghosted zone.
         */
        if (curr_entity->processor_id() == my_processor_id ||
            neighbor->processor_id() == my_processor_id)
        {
          /**
           * Premark neighboring entities with a halo mark. These
           * entities may or may not end up being part of the feature.
           * We will not update the _entities_visited data structure
           * here.
           */
          if (topological_neighbor || disjoint_only)
            feature->_disjoint_halo_ids.insert(feature->_disjoint_halo_ids.end(), neighbor->id());
          else
            _entity_queue.push_front(neighbor);
        }
      }
    }
  }
}

void
FeatureFloodCount::appendPeriodicNeighborNodes(FeatureData & feature) const
{
  if (_is_elemental)
  {
    for (auto entity : feature._local_ids)
    {
      Elem * elem = _mesh.elemPtr(entity);

      for (auto node_n = decltype(elem->n_nodes())(0); node_n < elem->n_nodes(); ++node_n)
      {
        auto iters = _periodic_node_map.equal_range(elem->node(node_n));

        for (auto it = iters.first; it != iters.second; ++it)
        {
          feature._periodic_nodes.insert(feature._periodic_nodes.end(), it->first);
          feature._periodic_nodes.insert(feature._periodic_nodes.end(), it->second);
        }
      }
    }
  }
  else
  {
    for (auto entity : feature._local_ids)
    {
      auto iters = _periodic_node_map.equal_range(entity);

      for (auto it = iters.first; it != iters.second; ++it)
      {
        feature._periodic_nodes.insert(feature._periodic_nodes.end(), it->first);
        feature._periodic_nodes.insert(feature._periodic_nodes.end(), it->second);
      }
    }
  }

  // TODO: Remove duplicates
}

template <typename T>
bool
FeatureFloodCount::isBoundaryEntity(const T * entity) const
{
  mooseAssert(_bnd_elem_range, "Boundary Element Range is nullptr");

  if (entity)
    for (const auto & belem : *_bnd_elem_range)
      // Only works for Elements
      if (belem->_elem->id() == entity->id() && hasBoundary(belem->_bnd_id))
        return true;

  return false;
}

void
FeatureFloodCount::FeatureData::updateBBoxExtremes(MeshBase & mesh)
{
  // First update the primary bounding box (all topologically connected)
  for (auto & halo_id : _halo_ids)
    updateBBoxExtremesHelper(_bboxes[0], *mesh.elem(halo_id));
  for (auto & ghost_id : _ghosted_ids)
    updateBBoxExtremesHelper(_bboxes[0], *mesh.elem(ghost_id));

  // Remove all of the IDs that are in the primary region
  std::list<dof_id_type> disjoint_elem_id_list;
  std::set_difference(_disjoint_halo_ids.begin(),
                      _disjoint_halo_ids.end(),
                      _halo_ids.begin(),
                      _halo_ids.end(),
                      std::insert_iterator<std::list<dof_id_type>>(disjoint_elem_id_list,
                                                                   disjoint_elem_id_list.begin()));

  if (!disjoint_elem_id_list.empty())
  {
    /**
     * Now we need to find how many distinct topologically disconnected sets of elements we have.
     * We've already removed elements that are part of the primary halo, we'll start by assuming
     * that element left is part of the same disjoint set. For each element, we'll see if it is a
     * neighbor of any other element in the current set. If it's not, then it must be part of yet
     * another set. The process repeats until every element is processed and put in the right
     * bucket.
     */
    std::list<std::list<dof_id_type>> disjoint_regions;
    for (auto elem_id : _disjoint_halo_ids)
    {
      disjoint_regions.emplace_back(std::list<dof_id_type>({elem_id}));
    }

    for (auto it1 = disjoint_regions.begin(); it1 != disjoint_regions.end(); /* No increment */)
    {
      bool merge_occured = false;
      for (auto it2 = disjoint_regions.begin(); it2 != disjoint_regions.end(); ++it2)
      {
        if (it1 != it2 && areElemListsMergeable(*it1, *it2, mesh))
        {
          it2->splice(it2->begin(), *it1);

          disjoint_regions.emplace_back(std::move(*it2));
          disjoint_regions.erase(it2);
          it1 = disjoint_regions.erase(it1);

          merge_occured = true;

          break;
        }
      }

      if (!merge_occured)
        ++it1;
    }

    // Finally create new bounding boxes for each disjoint region
    auto num_regions = disjoint_regions.size();
    // We have num_regions *new* bounding boxes plus the existing bounding box
    _bboxes.resize(num_regions + 1);

    decltype(num_regions) region = 1;
    for (const auto list_ref : disjoint_regions)
    {
      for (const auto elem_id : list_ref)
        updateBBoxExtremesHelper(_bboxes[region], *mesh.elem_ptr(elem_id));

      FeatureData::container_type set_union;
      FeatureFloodCount::reserve(set_union, _halo_ids.size() + _disjoint_halo_ids.size());
      std::set_union(
          _halo_ids.begin(),
          _halo_ids.end(),
          _disjoint_halo_ids.begin(),
          _disjoint_halo_ids.end(),
          std::insert_iterator<FeatureData::container_type>(set_union, set_union.begin()));
      _halo_ids.swap(set_union);

      _disjoint_halo_ids.clear();
      ++region;
    }
  }
}

void
FeatureFloodCount::FeatureData::updateBBoxExtremes(MeshTools::BoundingBox & bbox,
                                                   const MeshTools::BoundingBox & rhs_bbox)
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
      if (bbox_lhs.intersects(bbox_rhs, libMesh::TOLERANCE * libMesh::TOLERANCE))
        return true;

  return false;
}

bool
FeatureFloodCount::FeatureData::halosIntersect(const FeatureData & rhs) const
{
  return setsIntersect(
      _halo_ids.begin(), _halo_ids.end(), rhs._halo_ids.begin(), rhs._halo_ids.end());
}

bool
FeatureFloodCount::FeatureData::periodicBoundariesIntersect(const FeatureData & rhs) const
{
  return setsIntersect(_periodic_nodes.begin(),
                       _periodic_nodes.end(),
                       rhs._periodic_nodes.begin(),
                       rhs._periodic_nodes.end());
}

bool
FeatureFloodCount::FeatureData::ghostedIntersect(const FeatureData & rhs) const
{
  return setsIntersect(
      _ghosted_ids.begin(), _ghosted_ids.end(), rhs._ghosted_ids.begin(), rhs._ghosted_ids.end());
}

bool
FeatureFloodCount::FeatureData::mergeable(const FeatureData & rhs) const
{
  return (_var_index == rhs._var_index &&      // the sets have matching variable indices and
          ((boundingBoxesIntersect(rhs) &&     //  (if the feature's bboxes intersect and
            ghostedIntersect(rhs))             //   the ghosted entities also intersect)
           ||                                  //   or
           periodicBoundariesIntersect(rhs))); //   periodic node sets intersect)
}

bool
FeatureFloodCount::FeatureData::canConsolidate(const FeatureData & rhs) const
{
  for (const auto & orig_id_pair1 : _orig_ids)
    for (const auto & orig_id_pair2 : rhs._orig_ids)
      if (orig_id_pair1 == orig_id_pair2)
        return true;

  return false;
}

void
FeatureFloodCount::FeatureData::merge(FeatureData && rhs)
{
  mooseAssert(_var_index == rhs._var_index, "Mismatched variable index in merge");
  mooseAssert(_id == rhs._id, "Mismatched auxiliary id in merge");

  FeatureData::container_type set_union;

  FeatureFloodCount::reserve(set_union, _local_ids.size() + rhs._local_ids.size());
  std::set_union(_local_ids.begin(),
                 _local_ids.end(),
                 rhs._local_ids.begin(),
                 rhs._local_ids.end(),
                 std::insert_iterator<FeatureData::container_type>(set_union, set_union.begin()));
  _local_ids.swap(set_union);

  set_union.clear();
  FeatureFloodCount::reserve(set_union, _periodic_nodes.size() + rhs._periodic_nodes.size());
  std::set_union(_periodic_nodes.begin(),
                 _periodic_nodes.end(),
                 rhs._periodic_nodes.begin(),
                 rhs._periodic_nodes.end(),
                 std::insert_iterator<FeatureData::container_type>(set_union, set_union.begin()));
  _periodic_nodes.swap(set_union);

  set_union.clear();
  FeatureFloodCount::reserve(set_union, _ghosted_ids.size() + rhs._ghosted_ids.size());
  std::set_union(_ghosted_ids.begin(),
                 _ghosted_ids.end(),
                 rhs._ghosted_ids.begin(),
                 rhs._ghosted_ids.end(),
                 std::insert_iterator<FeatureData::container_type>(set_union, set_union.begin()));

  /**
   * Even though we've determined that these two partial regions need to be merged, we don't
   * necessarily know if the _ghost_ids intersect. We could be in this branch because the periodic
   * boundaries intersect but that doesn't tell us anything about whether or not the ghost_region
   * also intersects. If the _ghost_ids intersect, that means that we are merging along a periodic
   * boundary, not across one. In this case the bounding box(s) need to be expanded.
   */
  bool physical_intersection = (_ghosted_ids.size() + rhs._ghosted_ids.size() > set_union.size());
  _ghosted_ids.swap(set_union);

  /**
   * If we had a physical intersection, we need to expand boxes. If we had a virtual (periodic)
   * intersection we need to preserve all of the boxes from each of the regions' sets.
   */
  if (physical_intersection)
    expandBBox(rhs);
  else
    std::move(rhs._bboxes.begin(), rhs._bboxes.end(), std::back_inserter(_bboxes));

  set_union.clear();
  FeatureFloodCount::reserve(set_union, _disjoint_halo_ids.size() + rhs._disjoint_halo_ids.size());
  std::set_union(_disjoint_halo_ids.begin(),
                 _disjoint_halo_ids.end(),
                 rhs._disjoint_halo_ids.begin(),
                 rhs._disjoint_halo_ids.end(),
                 std::insert_iterator<FeatureData::container_type>(set_union, set_union.begin()));
  _disjoint_halo_ids.swap(set_union);

  set_union.clear();
  FeatureFloodCount::reserve(set_union, _halo_ids.size() + rhs._halo_ids.size());
  std::set_union(_halo_ids.begin(),
                 _halo_ids.end(),
                 rhs._halo_ids.begin(),
                 rhs._halo_ids.end(),
                 std::insert_iterator<FeatureData::container_type>(set_union, set_union.begin()));
  _halo_ids.swap(set_union);

  // Keep track of the original ids so we can notify other processors of the local to global mapping
  _orig_ids.splice(_orig_ids.end(), std::move(rhs._orig_ids));

  // Update the min feature id
  _min_entity_id = std::min(_min_entity_id, rhs._min_entity_id);

  /**
   * Combine the status flags: Currently we only expect to combine CLEAR and INACTIVE. Any other
   * combination is currently a logic error. In this case of CLEAR and INACTIVE though,
   * we want to make sure that CLEAR wins.
   */
  mooseAssert((_status & Status::MARKED & Status::DIRTY) == Status::CLEAR,
              "Flags in invalid state");

  // Logical AND here to combine flags (INACTIVE & INACTIVE == INACTIVE, all other combos are CLEAR)
  _status &= rhs._status;

  // Logical OR here to make sure we maintain boundary intersection attribute when joining
  _intersects_boundary |= rhs._intersects_boundary;

  _vol_count += rhs._vol_count;
  _centroid += rhs._centroid;
}

void
FeatureFloodCount::FeatureData::consolidate(FeatureData && rhs)
{
  mooseAssert(_var_index == rhs._var_index, "Mismatched variable index in merge");
  mooseAssert(_id == rhs._id, "Mismatched auxiliary id in merge");

  FeatureData::container_type set_union;
  FeatureFloodCount::reserve(_local_ids, _local_ids.size() + rhs._local_ids.size());
  std::set_union(_local_ids.begin(),
                 _local_ids.end(),
                 rhs._local_ids.begin(),
                 rhs._local_ids.end(),
                 std::insert_iterator<FeatureData::container_type>(set_union, set_union.begin()));
  _local_ids.swap(set_union);

  mooseAssert((_status & Status::MARKED & Status::DIRTY) == Status::CLEAR,
              "Flags in invalid state");
}

void
FeatureFloodCount::FeatureData::clear()
{
  _local_ids.clear();
  _periodic_nodes.clear();
  _halo_ids.clear();
  _disjoint_halo_ids.clear();
  _ghosted_ids.clear();
  _bboxes.clear();
  _orig_ids.clear();
}

void
FeatureFloodCount::FeatureData::expandBBox(const FeatureData & rhs)
{
  std::vector<bool> intersected_boxes(rhs._bboxes.size(), false);

  auto box_expanded = false;
  for (auto & bbox : _bboxes)
    for (auto j = beginIndex(rhs._bboxes); j < rhs._bboxes.size(); ++j)
    {
      if (bbox.intersects(rhs._bboxes[j], libMesh::TOLERANCE * libMesh::TOLERANCE))
      {
        updateBBoxExtremes(bbox, rhs._bboxes[j]);
        intersected_boxes[j] = true;
        box_expanded = true;
      }
    }

  // Error check
  if (!box_expanded)
  {
    std::ostringstream oss;
    oss << "LHS BBoxes:\n";
    for (auto i = beginIndex(_bboxes); i < _bboxes.size(); ++i)
      oss << "Max: " << _bboxes[i].max() << " Min: " << _bboxes[i].min() << '\n';

    oss << "RHS BBoxes:\n";
    for (auto i = beginIndex(rhs._bboxes); i < rhs._bboxes.size(); ++i)
      oss << "Max: " << rhs._bboxes[i].max() << " Min: " << rhs._bboxes[i].min() << '\n';

    ::mooseError("No Bounding Boxes Expanded - This is a catastrophic error!\n", oss.str());
  }

  // Any bounding box in the rhs vector that doesn't intersect
  // needs to be appended to the lhs vector
  for (auto j = beginIndex(intersected_boxes); j < intersected_boxes.size(); ++j)
    if (!intersected_boxes[j])
      _bboxes.push_back(rhs._bboxes[j]);
}

std::ostream &
operator<<(std::ostream & out, const FeatureFloodCount::FeatureData & feature)
{
  static const bool debug = true;

  out << "Grain ID: ";
  if (feature._id != FeatureFloodCount::invalid_id)
    out << feature._id;
  else
    out << "invalid";

  if (debug)
  {
    out << "\nGhosted Entities: ";
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
              (MooseUtils::absoluteFuzzyEqual(bbox.max()(2), bbox.min()(2))
                   ? 1
                   : bbox.max()(2) - bbox.min()(2));
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
    out << "\nVar_index: " << feature._var_index;
    out << "\nMin Entity ID: " << feature._min_entity_id;
  }
  out << "\n\n";

  return out;
}

/*****************************************************************************************
 ******************************* Utility Routines ****************************************
 *****************************************************************************************
 */
void
updateBBoxExtremesHelper(MeshTools::BoundingBox & bbox, const Point & node)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    bbox.min()(i) = std::min(bbox.min()(i), node(i));
    bbox.max()(i) = std::max(bbox.max()(i), node(i));
  }
}

void
updateBBoxExtremesHelper(MeshTools::BoundingBox & bbox, const Elem & elem)
{
  for (auto node_n = decltype(elem.n_nodes())(0); node_n < elem.n_nodes(); ++node_n)
    updateBBoxExtremesHelper(bbox, *(elem.get_node(node_n)));
}

bool
areElemListsMergeable(const std::list<dof_id_type> & elem_list1,
                      const std::list<dof_id_type> & elem_list2,
                      MeshBase & mesh)
{
  for (const auto elem_id1 : elem_list1)
  {
    const auto * elem1 = mesh.elem_ptr(elem_id1);
    for (const auto elem_id2 : elem_list2)
    {
      const auto * elem2 = mesh.elem_ptr(elem_id2);
      if (elem1->has_neighbor(elem2))
        return true;
    }
  }
  return false;
}

// Constants
const std::size_t FeatureFloodCount::invalid_size_t = std::numeric_limits<std::size_t>::max();
const unsigned int FeatureFloodCount::invalid_id = std::numeric_limits<unsigned int>::max();
