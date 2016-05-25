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

// TODO: Replace this with something better that can handle MooseSharedPointer<T>
template<typename T>
struct DereferenceSorter
{
  bool operator()(const T & lhs, const T & rhs) const
  {
    return *lhs < *rhs;
  }
};

template<>
void dataStore(std::ostream & stream, FeatureFloodCount::FeatureData & feature, void * context)
{
  storeHelper(stream, feature._ghosted_ids, context);
  storeHelper(stream, feature._halo_ids, context);
  storeHelper(stream, feature._periodic_nodes, context);
  storeHelper(stream, feature._var_idx, context);
  storeHelper(stream, feature._bboxes, context);
  storeHelper(stream, feature._min_entity_id, context);
  storeHelper(stream, feature._status, context);
  storeHelper(stream, feature._merged, context);
  storeHelper(stream, feature._intersects_boundary, context);
}

template<>
void dataStore(std::ostream & stream, MooseSharedPointer<FeatureFloodCount::FeatureData> & feature, void * context)
{
  dataStore(stream, *feature, context);
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
  loadHelper(stream, feature._ghosted_ids, context);
  loadHelper(stream, feature._halo_ids, context);
  loadHelper(stream, feature._periodic_nodes, context);
  loadHelper(stream, feature._var_idx, context);
  loadHelper(stream, feature._bboxes, context);
  loadHelper(stream, feature._min_entity_id, context);
  loadHelper(stream, feature._status, context);
  loadHelper(stream, feature._merged, context);
  loadHelper(stream, feature._intersects_boundary, context);
}

template<>
void dataLoad(std::istream & stream, MooseSharedPointer<FeatureFloodCount::FeatureData> & feature, void * context)
{
  feature = MooseSharedPointer<FeatureFloodCount::FeatureData>(new FeatureFloodCount::FeatureData());

  dataLoad(stream, *feature, context);
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
    _maps_size(_single_map_mode ? 1 : _vars.size()),
    _entities_visited(_vars.size()), // This map is always sized to the number of variables
    _feature_count(0),
    _partial_feature_sets(_app.n_processors()),
    _feature_sets(_maps_size),
    _feature_maps(_maps_size),
    _pbs(NULL),
    _element_average_value(parameters.isParamValid("elem_avg_value") ? getPostprocessorValue("elem_avg_value") : _real_zero),
    _compute_boundary_intersecting_volume(getParam<bool>("compute_boundary_intersecting_volume")),
    _is_elemental(getParam<MooseEnum>("flood_entity_type") == "ELEMENTAL" ? true : false)
{
  // Size the data structures to hold the correct number of maps
  for (unsigned int rank = 0; rank < _app.n_processors(); ++rank)
    _partial_feature_sets[rank].resize(_maps_size);

  if (_var_index_mode)
    _var_index_maps.resize(_maps_size);
}

FeatureFloodCount::~FeatureFloodCount()
{
}

void
FeatureFloodCount::initialize()
{
  // Get a pointer to the PeriodicBoundaries buried in libMesh
  // TODO: Can we do this in the constructor (i.e. are all objects necessary for this call in existance during ctor?)
  _pbs = _fe_problem.getNonlinearSystem().dofMap().get_periodic_boundaries();

  // Clear the bubble marking maps and region counters and other data structures
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    _feature_maps[map_num].clear();
    _feature_sets[map_num].clear();

    if (_var_index_mode)
      _var_index_maps[map_num].clear();
  }

  // TODO: use iterator
  for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
    _entities_visited[var_num].clear();

  // Build a new node to element map
  _nodes_to_elem_map.clear();
  MeshTools::build_nodes_to_elem_map(_mesh.getMesh(), _nodes_to_elem_map);

  // TODO: We might only need to build this once if adaptivity is turned off
  _mesh.buildPeriodicNodeMap(_periodic_node_map, _var_number, _pbs);

  // Calculate the thresholds for this iteration
  _step_threshold = _element_average_value + _threshold;
  _step_connecting_threshold = _element_average_value + _connecting_threshold;

  _all_feature_volumes.clear();

  _ghosted_entity_ids.clear();

  // Reset the feature count
  _feature_count = 0;
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
      for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
        flood(current_elem, var_num, NULL /* Designates inactive feature */);
    }
    else
    {
      unsigned int n_nodes = current_elem->n_vertices();
      for (unsigned int i = 0; i < n_nodes; ++i)
      {
        const Node * current_node = current_elem->get_node(i);

        for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
          flood(current_node, var_num, NULL /* Designates inactive feature */);
      }
    }
  }
}

void FeatureFloodCount::communicateAndMerge()
{
  // First we need to transform the raw data into a usable data structure
  populateDataStructuresFromFloodData();

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
  _communicator.allgather_packed_range((void *)(NULL), send_buffers.begin(), send_buffers.end(),
                                       std::back_inserter(recv_buffers));

  deserialize(recv_buffers);

  /*********************************************************************************
   *********************************************************************************
   * End Parallel Communication Section
   *********************************************************************************
   *********************************************************************************/

  // We'll inflate the bounding boxes by a percentage of the domain
  RealVectorValue inflation;
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    inflation(i) = _mesh.dimensionWidth(i);

  // Let's try 1%
  inflation *= 0.01;
  inflateBoundingBoxes(inflation);

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
FeatureFloodCount::getEntityValue(dof_id_type entity_id, FIELD_TYPE field_type, unsigned int var_idx) const
{
  mooseAssert(var_idx < _maps_size, "Index out of range");

  switch (field_type)
  {
    case UNIQUE_REGION:
    {
      std::map<dof_id_type, int>::const_iterator entity_it = _feature_maps[var_idx].find(entity_id);

      if (entity_it != _feature_maps[var_idx].end())
        return entity_it->second; // + _region_offsets[var_idx];
      else
        return -1;
    }

    case VARIABLE_COLORING:
    {
      std::map<dof_id_type, int>::const_iterator entity_it = _var_index_maps[var_idx].find(entity_id);

      if (entity_it != _var_index_maps[var_idx].end())
        return entity_it->second;
      else
        return -1;
    }

    case GHOSTED_ENTITIES:
    {
      std::map<dof_id_type, int>::const_iterator entity_it = _ghosted_entity_ids.find(entity_id);

      if (entity_it != _ghosted_entity_ids.end())
        return entity_it->second;
      else
        return -1;
    }

    case HALOS:
    {
      std::map<dof_id_type, int>::const_iterator entity_it = _halo_ids.find(entity_id);

      if (entity_it != _halo_ids.end())
        return entity_it->second;
      else
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

// TODO: Possibly rename this routine
void
FeatureFloodCount::populateDataStructuresFromFloodData()
{
  MeshBase & mesh = _mesh.getMesh();
  processor_id_type n_procs = _app.n_processors();

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    for (processor_id_type rank = 0; rank < n_procs; ++rank)
      for (std::vector<FeatureData>::iterator it = _partial_feature_sets[rank][map_num].begin();
           it != _partial_feature_sets[rank][map_num].end(); ++it)
      {
        FeatureData & feature = *it;

        for (std::set<dof_id_type>::iterator entity_it = feature._local_ids.begin(); entity_it != feature._local_ids.end(); ++entity_it)
        {
          dof_id_type entity_id = *entity_it;

          // TODO: This may not be good enough for the elemental case
          const Point & entity_point = _is_elemental ? mesh.elem(entity_id)->centroid() : mesh.node(entity_id);

          /**
           * Update the bounding box.
           *
           * Note: There will always be one and only one bbox while we are building up our
           * data structures because we haven't started to stitch together any regions yet.
           */
          feature.updateBBoxMin(feature._bboxes[0], entity_point);
          feature.updateBBoxMax(feature._bboxes[0], entity_point);

          // Save off the min entity id present in the feature to uniquely identify the feature regardless of n_procs
          feature._min_entity_id = std::min(feature._min_entity_id, entity_id);
        }

        // Adjust the halo marking region
        std::set<dof_id_type> set_difference;

        std::set_difference(feature._halo_ids.begin(), feature._halo_ids.end(), feature._local_ids.begin(), feature._local_ids.end(),
                            std::insert_iterator<std::set<dof_id_type> >(set_difference, set_difference.begin()));
        feature._halo_ids.swap(set_difference);

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
  dataStore(oss, _partial_feature_sets[processor_id()], this);

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

  for (unsigned int rank = 0; rank < serialized_buffers.size(); ++rank)
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
    dataLoad(iss, _partial_feature_sets[rank], this);
  }
}

void
FeatureFloodCount::mergeSets(bool use_periodic_boundary_info)
{
  Moose::perf_log.push("mergeSets()", "FeatureFloodCount");
  std::set<dof_id_type> set_union;

  processor_id_type n_procs = _app.n_processors();

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    for (processor_id_type rank1 = 0; rank1 < n_procs; ++rank1)
    {
    REPEAT_MERGE_LOOPS:

      for (processor_id_type rank2 = 0; rank2 < n_procs; ++rank2)
      {
        for (std::vector<FeatureData>::iterator it1 = _partial_feature_sets[rank1][map_num].begin();
             it1 != _partial_feature_sets[rank1][map_num].end(); /* no increment ++it1 */)
        {
          if (it1->_merged)
          {
            ++it1;
            continue;
          }

          bool region_merged = false;
          for (std::vector<FeatureData>::iterator it2 = _partial_feature_sets[rank2][map_num].begin(); it2 != _partial_feature_sets[rank2][map_num].end(); ++it2)
          {
            bool pb_intersect = false;
            if (it1 != it2 &&                                                                // Make sure that these iterators aren't pointing at the same set
                !it2->_merged &&                                                             // and that it2 is not merged (it1 was already checked)
                it1->_var_idx == it2->_var_idx &&                                            // and that the sets have matching variable indices
                ((use_periodic_boundary_info &&                                              // and (if merging across periodic nodes
                   (pb_intersect = setsIntersect(it1->_periodic_nodes.begin(),               //      do those periodic nodes intersect?
                                                 it1->_periodic_nodes.end(),
                                                 it2->_periodic_nodes.begin(),
                                                 it2->_periodic_nodes.end())))
                     ||                                                                      //      or
                   (it1->isStichable(*it2) &&                                                //      if the region bboxes intersect
                     (setsIntersect(it1->_ghosted_ids.begin(),                               //      do those ghosted nodes intersect?)
                                    it1->_ghosted_ids.end(),
                                    it2->_ghosted_ids.begin(),
                                    it2->_ghosted_ids.end()))
                   )
                 )
               )
            {
              /**
               * Even though we've determined that these two partial regions need to be merged, we don't necessarily know if the _ghost_ids intersect.
               * We could be in this branch because the periodic boundaries intersect but that doesn't tell us anything about whether or not the ghost_region
               * also intersects. If the _ghost_ids intersect, that means that we are merging along a periodic boundary, not across one. In this case the
               * bounding box(s) need to be expanded.
               */
              set_union.clear();
              std::set_union(it1->_ghosted_ids.begin(), it1->_ghosted_ids.end(), it2->_ghosted_ids.begin(), it2->_ghosted_ids.end(),
                             std::insert_iterator<std::set<dof_id_type> >(set_union, set_union.begin()));

              // Was there overlap in the physical region?
              bool physical_intersection = (it1->_ghosted_ids.size() + it2->_ghosted_ids.size() > set_union.size());

              it1->_ghosted_ids.swap(set_union);
              it2->_ghosted_ids.clear();

              set_union.clear();
              std::set_union(it1->_periodic_nodes.begin(), it1->_periodic_nodes.end(), it2->_periodic_nodes.begin(), it2->_periodic_nodes.end(),
                             std::insert_iterator<std::set<dof_id_type> >(set_union, set_union.begin()));
              it1->_periodic_nodes.swap(set_union);
              it2->_periodic_nodes.clear();

              set_union.clear();
              std::set_union(it1->_local_ids.begin(), it1->_local_ids.end(), it2->_local_ids.begin(), it2->_local_ids.end(),
                             std::insert_iterator<std::set<dof_id_type> >(set_union, set_union.begin()));
              it1->_local_ids.swap(set_union);
              it2->_local_ids.clear();

              set_union.clear();
              std::set_union(it1->_halo_ids.begin(), it1->_halo_ids.end(), it2->_halo_ids.begin(), it2->_halo_ids.end(),
                             std::insert_iterator<std::set<dof_id_type> >(set_union, set_union.begin()));
              it1->_halo_ids.swap(set_union);
              it2->_halo_ids.clear();

              /**
               * If we had a physical intersection, we need to expand boxes. If we had a virtual (periodic) intersection we need to preserve
               * all of the boxes from each of the regions' sets.
               */
              if (physical_intersection)
                it1->expandBBox(*it2);
              else
                std::copy(it2->_bboxes.begin(), it2->_bboxes.end(), std::back_inserter(it1->_bboxes));

              // Update the min feature id
              it1->_min_entity_id = std::min(it1->_min_entity_id, it2->_min_entity_id);

              // Set the flag on the merged set so we don't revisit it again
              it2->_merged = true;

              // Something was merged so we'll need to repeat this loop
              region_merged = true;
            }
          } // it2 loop

          // Don't increment if we had a merge, we need to retry earlier candidates again
          if (!region_merged)
            ++it1;
          else
            goto REPEAT_MERGE_LOOPS;

        } // it1 loop
      } // rank2 loop
    } // rank1 loop
  } // map loop


  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    _feature_sets[map_num].clear();

  // Now consolidate the data structure
  _feature_count = 0;
  for (processor_id_type rank = 0; rank < n_procs; ++rank)
    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    {
      for (std::vector<FeatureData>::iterator it = _partial_feature_sets[rank][map_num].begin();
           it != _partial_feature_sets[rank][map_num].end(); ++it)
      {
        if (!it->_merged)
        {
          _feature_sets[map_num].push_back(MooseSharedPointer<FeatureData>(new FeatureData(*it)));
          ++_feature_count;
        }
      }

      _partial_feature_sets[rank][map_num].clear();
    }

  Moose::perf_log.pop("mergeSets()", "FeatureFloodCount");
}

void
FeatureFloodCount::updateFieldInfo()
{
  // This variable is only relevant in single map mode
//  _region_to_var_idx.resize(_feature_sets[0].size());
  _halo_ids.clear();

  std::vector<size_t> index_vector;

  unsigned int feature_number = 0;
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    /**
     * Perform an indirect sort to give a parallel unique sorting to the identified features.
     * We use the "min_entity_id" inside each feature to assign it's position in the
     * sorted indices vector.
     */
    Moose::indirectSort(_feature_sets[map_num].begin(), _feature_sets[map_num].end(), index_vector, DereferenceSorter<MooseSharedPointer<FeatureData> >());

    // Clear out the original markings since they aren't unique globally
    _feature_maps[map_num].clear();

    // If the developer has requested _condense_map_info we'll make sure we only update the zeroth map
    unsigned int map_idx = (_single_map_mode || _condense_map_info) ? 0 : map_num;
    for (std::vector<size_t>::const_iterator idx_it = index_vector.begin(); idx_it != index_vector.end(); ++idx_it)
    {
      const FeatureData & feature = *_feature_sets[map_num][*idx_it];

      // Loop over the entitiy ids of this feature and update our local map
      for (std::set<dof_id_type>::const_iterator entity_it = feature._local_ids.begin(); entity_it != feature._local_ids.end(); ++entity_it)
      {
        _feature_maps[map_idx][*entity_it] = feature_number;

        if (_var_index_mode)
          _var_index_maps[map_idx][*entity_it] = feature._var_idx;
      }

      // Loop over the halo ids to update cells with halo information
      for (std::set<dof_id_type>::const_iterator entity_it = feature._halo_ids.begin(); entity_it != feature._halo_ids.end(); ++entity_it)
        _halo_ids[*entity_it] = feature_number;

      // Loop over the ghosted ids to update cells with ghost information
      for (std::set<dof_id_type>::const_iterator entity_it = feature._ghosted_ids.begin(); entity_it != feature._ghosted_ids.end(); ++entity_it)
        _ghosted_entity_ids[*entity_it] = feature_number;

      ++feature_number;
    }

    // If the user doesn't want a global numbering, we'll reset the feature_number for each map
    if (!_global_numbering)
      feature_number = 0;
  }

  mooseAssert(_feature_count == feature_number, "feature_number does not agree with previously calculated _feature_count");
}

void
FeatureFloodCount::flood(const DofObject * dof_object, int current_idx, FeatureData * feature)
{
  if (dof_object == NULL)
    return;

  // Retrieve the id of the current entity
  dof_id_type entity_id = dof_object->id();

  // Has this entity already been marked? - if so move along
  if (_entities_visited[current_idx].find(entity_id) != _entities_visited[current_idx].end())
    return;

  // Mark this entity as visited
  _entities_visited[current_idx][entity_id] = true;

  // Determine which threshold to use based on whether this is an established region
  Real threshold = feature ? _step_connecting_threshold : _step_threshold;

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
  unsigned int map_num = _single_map_mode ? 0 : current_idx;
  processor_id_type rank = processor_id();

  // New Feature (we need to create it and add it to our data structure)
  if (!feature)
    _partial_feature_sets[rank][map_num].push_back(FeatureData(current_idx));

  // Get a handle to the feature we will update (always the last feature in the data structure)
  feature = &_partial_feature_sets[rank][map_num].back();

  // Insert the current entity into the local ids map
  feature->_local_ids.insert(entity_id);

  if (_is_elemental)
    visitElementalNeighbors(static_cast<const Elem *>(dof_object), current_idx, feature, /*recurse =*/true);
  else
    visitNodalNeighbors(static_cast<const Node *>(dof_object), current_idx, feature, /*recurse =*/true);
}

void
FeatureFloodCount::visitElementalNeighbors(const Elem * elem, int current_idx, FeatureData * feature, bool recurse)
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

  // Loop over all active element neighbors
  for (std::vector<const Elem *>::const_iterator neighbor_it = all_active_neighbors.begin();
       neighbor_it != all_active_neighbors.end(); ++neighbor_it)
  {
    const Elem * neighbor = *neighbor_it;
    processor_id_type my_proc_id = processor_id();

    // Only recurse on elems this processor can see
    if (neighbor && neighbor->is_semilocal(my_proc_id))
    {
      if (neighbor->processor_id() != my_proc_id)
        feature->_ghosted_ids.insert(elem->id());

      /**
       * Premark neighboring entities with a halo mark. These
       * entities may or may not end up being part of the feature.
       * We will not update the _entities_visited data structure
       * here.
       */
      feature->_halo_ids.insert(neighbor->id());

      if (recurse)
        flood(neighbor, current_idx, feature);
    }
  }
}

void
FeatureFloodCount::visitNodalNeighbors(const Node * node, int current_idx, FeatureData * feature, bool recurse)
{
  mooseAssert(node, "Node is NULL");

  std::vector<const Node *> neighbors;
  MeshTools::find_nodal_neighbors(_mesh.getMesh(), *node, _nodes_to_elem_map, neighbors);

  // Loop over all nodal neighbors
  for (unsigned int i = 0; i < neighbors.size(); ++i)
  {
    const Node * neighbor_node = neighbors[i];
    processor_id_type my_proc_id = processor_id();

    // Only recurse on nodes this processor can see
    if (_mesh.isSemiLocal(const_cast<Node *>(neighbor_node)))
    {
      if (neighbor_node->processor_id() != my_proc_id)
        feature->_ghosted_ids.insert(neighbor_node->id());

      // Premark Halo values
      feature->_halo_ids.insert(neighbor_node->id());

      if (recurse)
        flood(neighbors[i], current_idx, feature);
    }
  }
}

void
FeatureFloodCount::appendPeriodicNeighborNodes(FeatureData & data) const
{
  typedef std::multimap<dof_id_type, dof_id_type>::const_iterator IterType;

  if (_is_elemental)
  {
    for (std::set<dof_id_type>::iterator entity_it = data._local_ids.begin(); entity_it != data._local_ids.end(); ++entity_it)
    {
      Elem * elem = _mesh.elemPtr(*entity_it);

      for (unsigned int node_n = 0; node_n < elem->n_nodes(); node_n++)
      {
        std::pair<IterType, IterType> iters = _periodic_node_map.equal_range(elem->node(node_n));

        for (IterType it = iters.first; it != iters.second; ++it)
        {
          data._periodic_nodes.insert(it->first);
          data._periodic_nodes.insert(it->second);
        }
      }
    }
  }
  else
  {
    for (std::set<dof_id_type>::iterator entity_it = data._local_ids.begin(); entity_it != data._local_ids.end(); ++entity_it)
    {
      std::pair<IterType, IterType> iters = _periodic_node_map.equal_range(*entity_it);

      for (IterType it = iters.first; it != iters.second; ++it)
      {
        data._periodic_nodes.insert(it->first);
        data._periodic_nodes.insert(it->second);
      }
    }
  }
}

void
FeatureFloodCount::inflateBoundingBoxes(RealVectorValue inflation_amount)
{
  processor_id_type n_procs = _app.n_processors();

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    for (processor_id_type rank = 0; rank < n_procs; ++rank)
      for (std::vector<FeatureData>::iterator it = _partial_feature_sets[rank][map_num].begin();
           it != _partial_feature_sets[rank][map_num].end(); ++it)
        it->inflateBoundingBoxes(inflation_amount);
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
    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    {
      std::vector<MooseSharedPointer<FeatureData> >::iterator
        bubble_it = _feature_sets[map_num].begin(),
        bubble_end = _feature_sets[map_num].end();

      // Determine boundary intersection for each FeatureData object
      for (; bubble_it != bubble_end; ++bubble_it)
        (*bubble_it)->_intersects_boundary = setsIntersect(all_boundary_node_ids.begin(), all_boundary_node_ids.end(),
                                                           (*bubble_it)->_local_ids.begin(), (*bubble_it)->_local_ids.end());
    }
  }

  // Size our temporary data structure
  std::vector<std::vector<Real> > bubble_volumes(_maps_size);
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
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
    unsigned int elem_n_nodes = elem->n_nodes();
    Real curr_volume = elem->volume();

    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    {
      std::vector<MooseSharedPointer<FeatureData> >::const_iterator
        bubble_it = _feature_sets[map_num].begin(),
        bubble_end = _feature_sets[map_num].end();

      for (unsigned int bubble_counter = 0; bubble_it != bubble_end; ++bubble_it, ++bubble_counter)
      {
        // Count the number of nodes on this element which are flooded.
        unsigned int flooded_nodes = 0;
        for (unsigned int node = 0; node < elem_n_nodes; ++node)
        {
          dof_id_type node_id = elem->node(node);
          if ((*bubble_it)->_local_ids.find(node_id) != (*bubble_it)->_local_ids.end())
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
          if ((*bubble_it)->_intersects_boundary)
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
    for (unsigned int i = 0; i < _total_volume_intersecting_boundary.size(); ++i)
      _total_volume_intersecting_boundary[i] /= total_volume;
  }

  // Stick all the partial bubble volumes in one long single vector to be gathered on the root processor
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    _all_feature_volumes.insert(_all_feature_volumes.end(), bubble_volumes[map_num].begin(), bubble_volumes[map_num].end());

  // do all the sums!
  _communicator.sum(_all_feature_volumes);

  std::sort(_all_feature_volumes.begin(), _all_feature_volumes.end(), std::greater<Real>());

  Moose::perf_log.pop("calculateBubbleVolume()", "FeatureFloodCount");
}

void
FeatureFloodCount::FeatureData::updateBBoxMin(MeshTools::BoundingBox & bbox, const Point & min)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    bbox.min()(i) = std::min(bbox.min()(i), min(i));
}

void
FeatureFloodCount::FeatureData::updateBBoxMax(MeshTools::BoundingBox & bbox, const Point & max)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    bbox.max()(i) = std::max(bbox.max()(i), max(i));
}

bool
FeatureFloodCount::FeatureData::isStichable(const FeatureData & rhs) const
{
  // See if any of the bounding boxes in either FeatureData object intersect
  for (unsigned int i = 0; i < _bboxes.size(); ++i)
    for (unsigned int j = 0; j < rhs._bboxes.size(); ++j)
      if (_bboxes[i].intersect(rhs._bboxes[j]))
        return true;

  return false;
}

void
FeatureFloodCount::FeatureData::expandBBox(const FeatureData & rhs)
{
  std::vector<bool> intersected_boxes(rhs._bboxes.size(), false);

  bool box_expanded = false;
  for (unsigned int i = 0; i < _bboxes.size(); ++i)
    for (unsigned int j = 0; j < rhs._bboxes.size(); ++j)
      if (_bboxes[i].intersect(rhs._bboxes[j]))
      {
        updateBBoxMin(_bboxes[i], rhs._bboxes[j].min());
        updateBBoxMax(_bboxes[i], rhs._bboxes[j].max());
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

    mooseError("Now Boundaing Boxes Expanded - This is a catastrophic error!\n" << oss.str());
  }
}

std::ostream &
operator<<(std::ostream & out, const FeatureFloodCount::FeatureData & feature)
{
  static const bool debug = true;

  if (debug)
  {
    out << "Ghosted Entities: ";
    for (std::set<dof_id_type>::const_iterator it = feature._ghosted_ids.begin(); it != feature._ghosted_ids.end(); ++it)
      out << *it << " ";

    out << "\nLocal Entities: ";
    for (std::set<dof_id_type>::const_iterator it = feature._local_ids.begin();  it != feature._local_ids.end(); ++it)
      out << *it << " ";

    out << "\nHalo Entities: ";
    for (std::set<dof_id_type>::const_iterator it = feature._halo_ids.begin();  it != feature._halo_ids.end(); ++it)
      out << *it << " ";

    out << "\nPeriodic Node IDs: ";
    for (std::set<dof_id_type>::const_iterator it = feature._periodic_nodes.begin();  it != feature._periodic_nodes.end(); ++it)
      out << *it << " ";
  }

  out << "\nBBoxes:";
  Real volume = 0;
  for (std::vector<MeshTools::BoundingBox>::const_iterator it = feature._bboxes.begin(); it != feature._bboxes.end(); ++it)
  {
    out << "\nMax: " << it->max() << " Min: " << it->min();
    volume += (it->max()(0) - it->min()(0)) * (it->max()(1) - it->min()(1)) *
      (MooseUtils::absoluteFuzzyEqual(it->max()(2), it->min()(2)) ? 1 : it->max()(2) - it->min()(2));
  }

  if (debug)
  {
    out << "\nVolume: " << volume;
    out << "\nVar_idx: " << feature._var_idx;
    out << "\nMin Entity ID: " << feature._min_entity_id;
    out << "\nMerge Flag: " << feature._merged;
  }
  out << "\n\n";

  return out;
}

const std::vector<std::pair<unsigned int, unsigned int> > FeatureFloodCount::_empty;
