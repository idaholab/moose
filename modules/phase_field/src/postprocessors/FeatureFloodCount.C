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
  storeHelper(stream, feature._min_entity_id, context);
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
  loadHelper(stream, feature._min_entity_id, context);
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
  params.addParam<bool>("use_single_map", true, "Determine whether information is tracked per coupled variable or consolidated into one (default: true)");
  params.addParam<bool>("condense_map_info", false, "Determines whether we condense all the node values when in multimap mode (default: false)");
  params.addParam<bool>("use_global_numbering", true, "Determine whether or not global numbers are used to label bubbles on multiple maps (default: true)");
  params.addParam<bool>("enable_var_coloring", false, "Instruct the UO to populate the variable index map.");
  params.addParam<bool>("use_less_than_threshold_comparison", true, "Controls whether bubbles are defined to be less than or greater than the threshold value.");
  params.addParam<FileName>("bubble_volume_file", "An optional file name where bubble volumes can be output.");
  params.addParam<bool>("compute_boundary_intersecting_volume", false, "If true, also compute the (normalized) volume of bubbles which intersect the boundary");
  params.set<bool>("use_displaced_mesh") = true;

  MooseEnum flood_type("NODAL ELEMENTAL", "NODAL");
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
    _mesh(_subproblem.mesh()),
    _var_number(_vars[0]->number()),
    _single_map_mode(getParam<bool>("use_single_map")),
    _condense_map_info(getParam<bool>("condense_map_info")),
    _global_numbering(getParam<bool>("use_global_numbering")),
    _var_index_mode(getParam<bool>("enable_var_coloring")),
    _use_less_than_threshold_comparison(getParam<bool>("use_less_than_threshold_comparison")),
    _n_vars(_vars.size()),
    _maps_size(_single_map_mode ? 1 : _vars.size()),
    _n_procs(_app.n_processors()),
    _entities_visited(_vars.size()), // This map is always sized to the number of variables
    _feature_count(0),
    _partial_feature_sets(_maps_size),
    _feature_sets(_maps_size),
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
    _feature_sets[map_num].clear();
    _partial_feature_sets[map_num].clear();

    if (_var_index_mode)
      _var_index_maps[map_num].clear();

    _halo_ids[map_num].clear();
  }

  for (auto var_num = decltype(_n_vars)(0); var_num < _vars.size(); ++var_num)
    _entities_visited[var_num].clear();

  // Calculate the thresholds for this iteration
  _step_threshold = _element_average_value + _threshold;
  _step_connecting_threshold = _element_average_value + _connecting_threshold;

  _all_feature_volumes.clear();

  _ghosted_entity_ids.clear();

  // Reset the feature count
  _feature_count = 0;
}

void
FeatureFloodCount::meshChanged()
{
  _mesh.buildPeriodicNodeMap(_periodic_node_map, _var_number, _pbs);

  // Build a new node to element map
  _nodes_to_elem_map.clear();
  MeshTools::build_nodes_to_elem_map(_mesh.getMesh(), _nodes_to_elem_map);
}

void
FeatureFloodCount::execute()
{
  const MeshBase::element_iterator end = _mesh.getMesh().active_local_elements_end();
  for (MeshBase::element_iterator el = _mesh.getMesh().active_local_elements_begin(); el != end; ++el)
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

  /*********************************************************************************
   *********************************************************************************
   * Begin Parallel Communication Section
   *********************************************************************************
   *********************************************************************************/

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
  recv_buffers.reserve(_app.n_processors());

  serialize(send_buffers[0]);

  /**
   * Each processor needs information from all other processors to create a complete
   * global feature map.
   */
  _communicator.allgather_packed_range((void *)(nullptr), send_buffers.begin(), send_buffers.end(),
                                       std::back_inserter(recv_buffers));

  deserialize(recv_buffers);

  /*********************************************************************************
   *********************************************************************************
   * End Parallel Communication Section
   *********************************************************************************
   *********************************************************************************/

  mergeSets(true);
}

void
FeatureFloodCount::finalize()
{
  communicateAndMerge();

  // Populate _feature_maps and _var_index_maps
  updateFieldInfo();

  // Calculate and out output bubble volume data
  if (_pars.isParamValid("bubble_volume_file"))
  {
    calculateBubbleVolumes();
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

  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
    for (auto & feature : _partial_feature_sets[map_num])
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

  for (auto rank = decltype(_n_procs)(0); rank < serialized_buffers.size(); ++rank)
  {
    /**
     * We should already have the local processor data in the features data structure.
     * Don't unpack the local buffer again.
     */
    if (rank == processor_id())
      continue;

    iss.str(serialized_buffers[rank]);    // populate the stream with a new buffer
    iss.clear();                          // reset the string stream state

    // Load the communicated data into all of the other processors' slots
    dataLoad(iss, _partial_feature_sets, this);
  }
}

void
FeatureFloodCount::mergeSets(bool use_periodic_boundary_info)
{
  Moose::perf_log.push("mergeSets()", "FeatureFloodCount");
  std::set<dof_id_type> set_union;

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
   * All of the merges are complete and stored in a vector of lists. To make several
   * of the sorting and tracking algorithms more straightforward, we will move these
   * items into a vector of vectors instead.
   */
  _feature_count = 0;
  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
  {
    for (auto & feature : _partial_feature_sets[map_num])
    {
      // Adjust the halo marking region
      std::set<dof_id_type> set_difference;
      std::set_difference(feature._halo_ids.begin(), feature._halo_ids.end(), feature._local_ids.begin(), feature._local_ids.end(),
                          std::insert_iterator<std::set<dof_id_type> >(set_difference, set_difference.begin()));
      feature._halo_ids.swap(set_difference);

      _feature_sets[map_num].emplace_back(std::move(feature));
      ++_feature_count;
    }

    _partial_feature_sets[map_num].clear();
  }

  Moose::perf_log.pop("mergeSets()", "FeatureFloodCount");
}

void
FeatureFloodCount::updateFieldInfo()
{
  // This variable is only relevant in single map mode
//  _region_to_var_idx.resize(_feature_sets[0].size());

  std::vector<size_t> index_vector;

  unsigned int feature_number = 0;
  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
  {
    /**
     * Perform an indirect sort to give a parallel unique sorting to the identified features.
     * We use the "min_entity_id" inside each feature to assign it's position in the
     * sorted indices vector.
     */
    Moose::indirectSort(_feature_sets[map_num].begin(), _feature_sets[map_num].end(), index_vector);

    // Clear out the original markings since they aren't unique globally
    _feature_maps[map_num].clear();

    // If the developer has requested _condense_map_info we'll make sure we only update the zeroth map
    auto map_idx = (_single_map_mode || _condense_map_info) ? decltype(map_num)(0) : map_num;
    for (auto idx : index_vector)
    {
      const auto & feature = _feature_sets[map_num][idx];

      // Loop over the entitiy ids of this feature and update our local map
      for (auto entity : feature._local_ids)
      {
        _feature_maps[map_idx][entity] = feature_number;

        if (_var_index_mode)
          _var_index_maps[map_idx][entity] = feature._var_idx;
      }

      // Loop over the halo ids to update cells with halo information
      for (auto entity : feature._halo_ids)
        _halo_ids[map_idx][entity] = feature_number;

      // Loop over the ghosted ids to update cells with ghost information
      for (auto entity : feature._ghosted_ids)
        _ghosted_entity_ids[entity] = 1;

      ++feature_number;
    }

    // If the user doesn't want a global numbering, we'll reset the feature_number for each map
    if (!_global_numbering)
      feature_number = 0;
  }

  mooseAssert(_feature_count == feature_number, "feature_number does not agree with previously calculated _feature_count");
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

  // Mark this entity as visited
  _entities_visited[current_idx][entity_id] = true;

  // Determine which threshold to use based on whether this is an established region
  auto threshold = feature ? _step_connecting_threshold : _step_threshold;

  // Get the value of the current variable for the current entity
  Real entity_value;
  if (_is_elemental)
  {
    const Elem * elem = static_cast<const Elem *>(dof_object);
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
   * we've found a new mesh entity that's part of a feature.
   */
  auto map_num = _single_map_mode ? decltype(current_idx)(0) : current_idx;

  // New Feature (we need to create it and add it to our data structure)
  if (!feature)
    _partial_feature_sets[map_num].emplace_back(current_idx);

  // Get a handle to the feature we will update (always the last feature in the data structure)
  feature = &_partial_feature_sets[map_num].back();

  // Insert the current entity into the local ids map
  feature->_local_ids.insert(entity_id);

  if (_is_elemental)
    visitElementalNeighbors(static_cast<const Elem *>(dof_object), current_idx, feature, /*expand_halos_only =*/false);
  else
    visitNodalNeighbors(static_cast<const Node *>(dof_object), current_idx, feature, /*expand_halos_only =*/false);
}

void
FeatureFloodCount::visitElementalNeighbors(const Elem * elem, unsigned long current_idx, FeatureData * feature, bool expand_halos_only)
{
  mooseAssert(elem, "Elem is NULL");

  std::vector<const Elem *> all_active_neighbors;

  // Loop over all neighbors (at the the same level as the current element)
  for (unsigned int i = 0; i < elem->n_neighbors(); ++i)
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

      for (unsigned int node_n = 0; node_n < elem->n_nodes(); node_n++)
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
FeatureFloodCount::calculateBubbleVolumes()
{
  Moose::perf_log.push("calculateBubbleVolume()", "FeatureFloodCount");

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

    // For each of the _maps_size FeatureData lists, determine if the set
    // of nodes includes any boundary nodes.
    for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
      // Determine boundary intersection for each FeatureData object
      for (auto & feature : _feature_sets[map_num])
        feature._intersects_boundary = setsIntersect(all_boundary_node_ids.begin(), all_boundary_node_ids.end(),
                                                     feature._local_ids.begin(), feature._local_ids.end());
  }

  // Size our temporary data structure
  std::vector<std::vector<Real> > bubble_volumes(_maps_size);
  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
    bubble_volumes[map_num].resize(_feature_sets[map_num].size());

  // Clear pre-existing values and allocate space to store the volume
  // of the boundary-intersecting grains for each variable.
  _total_volume_intersecting_boundary.clear();
  _total_volume_intersecting_boundary.resize(_maps_size);

  // Loop over the active local elements.  For each variable, and for
  // each FeatureData object, check whether a majority of the element's
  // nodes belong to that Bubble, and if so assign the element's full
  // volume to that bubble.
  const MeshBase::const_element_iterator el_end = _mesh.getMesh().active_local_elements_end();
  for (MeshBase::const_element_iterator el = _mesh.getMesh().active_local_elements_begin(); el != el_end; ++el)
  {
    Elem * elem = *el;
    auto elem_n_nodes = elem->n_nodes();
    auto curr_volume = elem->volume();

    for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
    {
      auto bubble_it = _feature_sets[map_num].cbegin();
      auto bubble_end = _feature_sets[map_num].cend();

      for (unsigned int bubble_counter = 0; bubble_it != bubble_end; ++bubble_it, ++bubble_counter)
      {
        // Count the number of nodes on this element which are flooded.
        unsigned int flooded_nodes = 0;
        for (auto node = decltype(elem_n_nodes)(0); node < elem_n_nodes; ++node)
        {
          auto node_id = elem->node(node);
          if ((*bubble_it)._local_ids.find(node_id) != (*bubble_it)._local_ids.end())
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
          if ((*bubble_it)._intersects_boundary)
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
    auto bbox = MeshTools::bounding_box(_mesh);
    auto total_volume = (bbox.max()(0)-bbox.min()(0))*(bbox.max()(1)-bbox.min()(1));

    // Sum up the partial boundary grain volume contributions from all processors
    _communicator.sum(_total_volume_intersecting_boundary);

    // Scale the boundary intersecting grain volumes by the total domain volume
    for (auto & total_volume_intersecting_boundary_item : _total_volume_intersecting_boundary)
      total_volume_intersecting_boundary_item /= total_volume;
  }

  // Stick all the partial bubble volumes in one long single vector to be gathered on the root processor
  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
    _all_feature_volumes.insert(_all_feature_volumes.end(), bubble_volumes[map_num].begin(), bubble_volumes[map_num].end());

  // do all the sums!
  _communicator.sum(_all_feature_volumes);

  std::sort(_all_feature_volumes.begin(), _all_feature_volumes.end(), std::greater<Real>());

  Moose::perf_log.pop("calculateBubbleVolume()", "FeatureFloodCount");
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
  for (unsigned int node_n = 0; node_n < elem.n_nodes(); ++node_n)
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

  // Update the min feature id
  _min_entity_id = std::min(_min_entity_id, rhs._min_entity_id);
}

void
FeatureFloodCount::FeatureData::expandBBox(const FeatureData & rhs)
{
  std::vector<bool> intersected_boxes(rhs._bboxes.size(), false);

  auto box_expanded = false;
  for (unsigned int i = 0; i < _bboxes.size(); ++i)
    for (unsigned int j = 0; j < rhs._bboxes.size(); ++j)
      if (_bboxes[i].intersect(rhs._bboxes[j]))
      {
        updateBBoxExtremes(_bboxes[i], rhs._bboxes[j]);
        intersected_boxes[j] = true;
        box_expanded = true;
      }

  // Any bounding box in the rhs vector that doesn't intersect
  // needs to be appended to the lhs vector
  for (unsigned int j = 0; j < intersected_boxes.size(); ++j)
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

  if (debug)
  {
    out << "\nVolume: " << volume;
    out << "\nVar_idx: " << feature._var_idx;
    out << "\nMin Entity ID: " << feature._min_entity_id;
  }
  out << "\n\n";

  return out;
}

const std::vector<std::pair<unsigned int, unsigned int> > FeatureFloodCount::_empty;
