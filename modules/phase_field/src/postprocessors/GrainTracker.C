/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// MOOSE includes
#include "GrainTracker.h"
#include "MooseMesh.h"
#include "GeneratedMesh.h"
#include "EBSDReader.h"
#include "NonlinearSystem.h"

// LibMesh includes
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/sphere.h"

#include <limits>
#include <algorithm>

template<>
InputParameters validParams<GrainTracker>()
{
  InputParameters params = validParams<FeatureFloodCount>();
  params += validParams<GrainTrackerInterface>();
  params.addClassDescription("Grain Tracker object for running reduced order parameter simulations without grain coalescence.");

  return params;
}

GrainTracker::GrainTracker(const InputParameters & parameters) :
    FeatureFloodCount(parameters),
    GrainTrackerInterface(),
    _tracking_step(getParam<int>("tracking_step")),
    _halo_level(getParam<unsigned int>("halo_level")),
    _n_reserve_ops(getParam<unsigned int>("reserve_op")),
    _reserve_op_idx(_n_reserve_ops <= _n_vars ? _n_vars - _n_reserve_ops : 0),
    _reserve_grain_first_idx(0),
    _reserve_op_threshold(getParam<Real>("reserve_op_threshold")),
    _remap(getParam<bool>("remap_grains")),
    _nl(static_cast<FEProblem &>(_subproblem).getNonlinearSystem()),
    _unique_grains(declareRestartableData<std::map<unsigned int, FeatureData> >("unique_grains")),
    _ebsd_reader(parameters.isParamValid("ebsd_reader") ? &getUserObject<EBSDReader>("ebsd_reader") : nullptr),
    _compute_op_maps(getParam<bool>("compute_op_maps"))
{
  if (!_is_elemental && _compute_op_maps)
    mooseError("\"compute_op_maps\" is only supported with \"flood_entity_type = ELEMENTAL\"");

  _empty_2.resize(_n_vars, libMesh::invalid_uint);
}

GrainTracker::~GrainTracker()
{
}

Real
GrainTracker::getEntityValue(dof_id_type node_id, FieldType field_type, unsigned int var_idx) const
{
  if (_t_step < _tracking_step)
    return 0;

  return FeatureFloodCount::getEntityValue(node_id, field_type, var_idx);
}

const std::vector<std::pair<unsigned int, unsigned int> > &
GrainTracker::getElementalValues(dof_id_type elem_id) const
{
  const auto pos = _elemental_data.find(elem_id);

  if (pos != _elemental_data.end())
    return pos->second;
  else
  {
#if DEBUG
    mooseDoOnce(Moose::out << "Elemental values not in structure for elem: " << elem_id << " this may be normal.");
#endif
    return _empty;
  }
}

unsigned int
GrainTracker::getNumberGrains() const
{
  return _feature_count;
}

Real
GrainTracker::getGrainVolume(unsigned int grain_id) const
{
  const auto feature_pair = _unique_grains.find(grain_id);
  mooseAssert(feature_pair != _unique_grains.end(), "Grain " << grain_id << " does not exist in data structure");

  return feature_pair->second._volume;
}

Point
GrainTracker::getGrainCentroid(unsigned int grain_id) const
{
  const auto feature_pair = _unique_grains.find(grain_id);
  mooseAssert(feature_pair != _unique_grains.end(), "Grain " << grain_id << " does not exist in data structure");

  return feature_pair->second._centroid;
}

void
GrainTracker::initialize()
{
  FeatureFloodCount::initialize();

  _elemental_data.clear();
  _elemental_data_2.clear();
}

void
GrainTracker::execute()
{
  Moose::perf_log.push("execute()", "GrainTracker");
  FeatureFloodCount::execute();
  Moose::perf_log.pop("execute()", "GrainTracker");
}

Real
GrainTracker::getThreshold(unsigned int current_idx, bool active_feature) const
{
  // If we are inspecting a reserve op parameter, we need to make sure
  // that there is an entity above the reserve_op threshold before
  // starting the flood of the feature.
  if (!active_feature && current_idx >= _reserve_op_idx)
    return _reserve_op_threshold;
  else
    return FeatureFloodCount::getThreshold(current_idx, active_feature);
}

void
GrainTracker::finalize()
{
  /**
   * Some perf_log operations appear here instead of inside of the named routines
   * because of multiple return paths.
   */

  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  Moose::perf_log.push("finalize()", "GrainTracker");

  expandHalos();

  // Build up the grain map on the root processor
  communicateAndMerge();

  /**
   * Track Grains
   */
  Moose::perf_log.push("trackGrains()", "GrainTracker");
  std::vector<unsigned int> new_grain_indices;

  // Track grains (only on the root processor)
  if (processor_id() == 0)
    trackGrains(new_grain_indices);

  if (processor_id() == 0)
  {
    std::cerr << "******************* Processor " << processor_id() << " *************************\n";
    for (auto & grain_pair : _unique_grains)
      std::cerr << grain_pair.second << '\n';
  }

  if (!new_grain_indices.empty())
  {
    // Communicate new grain indices with the remaining processors
    auto new_grains_count = new_grain_indices.size();
    _communicator.broadcast(new_grains_count);

    new_grain_indices.resize(new_grains_count);
    _communicator.broadcast(new_grain_indices);

    // Trigger the callback on all processors
    for (auto idx : new_grain_indices)
      newGrainCreated(idx);
  }

  std::vector<std::vector<unsigned int> > local_to_global_all;
  if (processor_id() == 0)
    buildLocalToGlobalIndices(local_to_global_all);

  scatterIndices(local_to_global_all);

  if (processor_id() != 0)
  {
    /**
     * On non-root processors we can't maintain the _unique_grains structure since we don't have all
     * of the global information. We clear it every time this object runs and populate
     * it with the information we do have.
     */
    _unique_grains.clear();

    for (auto & list_ref : _partial_feature_sets)
      for (auto && feature : list_ref)
      {
        mooseAssert(feature._orig_ids.size() == 1, "feature._orig_ids length doesn't make sense");
        auto local_id = feature._orig_ids.begin()->second;

        mooseAssert(local_id < _local_to_global_feature_map.size(), "local_id : " << local_id << " is out of range (" << _local_to_global_feature_map.size() << ')');
        _unique_grains.emplace(std::pair<unsigned int, FeatureData>(_local_to_global_feature_map[local_id], std::move(feature)));
      }

    std::cerr << "******************* Processor " << processor_id() << " *************************\n";
    for (auto & grain_pair : _unique_grains)
      std::cerr << grain_pair.second << '\n';
  }
  Moose::perf_log.pop("trackGrains()", "GrainTracker");

  _console << "Finished inside of trackGrains" << std::endl;

  Moose::perf_log.push("remapGrains()", "GrainTracker");
  if (_remap)
    remapGrains();
  Moose::perf_log.pop("remapGrains()", "GrainTracker");

  updateFieldInfo();

  _console << "Finished inside of updateFieldInfo" << std::endl;

  writeFeatureVolumeFile();

  if (_compute_op_maps)
  {
    for (const auto & grain_pair : _unique_grains)
    {
      if (grain_pair.second._status != Status::INACTIVE)
      {
        for (auto elem_id : grain_pair.second._local_ids)
        {
          mooseAssert(!_ebsd_reader || _unique_grain_to_ebsd_num.find(grain_pair.first) !=
                      _unique_grain_to_ebsd_num.end(), "Bad mapping in unique_grain_to_ebsd_num");
          _elemental_data[elem_id].emplace_back(_ebsd_reader ?
                                                _unique_grain_to_ebsd_num[grain_pair.first] :
                                                grain_pair.first,
                                                grain_pair.second._var_idx);

          auto data_pair = _elemental_data_2.find(elem_id);
          if (data_pair == _elemental_data_2.end())
          {
            auto data_pair_pair = _elemental_data_2.emplace(elem_id, std::vector<unsigned int>(_n_vars, libMesh::invalid_uint));
            data_pair = data_pair_pair.first;

            // insert the reserve op numbers (if appropriate)
            for (unsigned int reserve_idx = 0; reserve_idx < _n_reserve_ops; ++reserve_idx)
              data_pair->second[reserve_idx] = _reserve_grain_first_idx + reserve_idx;
          }
          data_pair->second[grain_pair.second._var_idx] = _ebsd_reader ? _unique_grain_to_ebsd_num[grain_pair.first] : grain_pair.first;
        }
      }
    }
  }

  _console << "Finished inside of GrainTracker" << std::endl;

  Moose::perf_log.pop("finalize()", "GrainTracker");
}

const std::vector<unsigned int> &
GrainTracker::getOpToGrainsVector(dof_id_type elem_id) const
{
  const auto pos = _elemental_data_2.find(elem_id);

  if (pos != _elemental_data_2.end())
    return pos->second;
  else
  {
#if DEBUG
    mooseDoOnce(Moose::out << "Elemental values not in structure for elem: " << elem_id << " this may be normal.");
#endif
    return _empty_2;
  }

}

void
GrainTracker::expandHalos()
{
  for (auto & list_ref  : _partial_feature_sets)
    for (auto & feature : list_ref)
    {
      for (auto halo_level = decltype(_halo_level)(1); halo_level < _halo_level; ++halo_level)
      {
        /**
         * Create a copy of the halo set so that as we insert new ids into the
         * set we don't continue to iterate on those new ids.
         */
        std::set<dof_id_type> orig_halo_ids(feature._halo_ids);

        for (auto entity : orig_halo_ids)
        {
          if (_is_elemental)
            visitElementalNeighbors(_mesh.elemPtr(entity), feature._var_idx, &feature, /*expand_halos_only =*/true);
          else
            visitNodalNeighbors(_mesh.nodePtr(entity), feature._var_idx, &feature, /*expand_halos_only =*/true);
        }
      }
    }
}

void
GrainTracker::trackGrains(std::vector<unsigned int> & new_grain_indices)
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  mooseAssert(processor_id() == 0, "trackGrains() should only be called on the root process");

  // Reset Status on active unique grains
  std::vector<unsigned int> map_sizes(_maps_size);
  for (auto & grain_pair : _unique_grains)
  {
    if (grain_pair.second._status != Status::INACTIVE)
    {
      grain_pair.second._status = Status::CLEAR;
      map_sizes[grain_pair.second._var_idx]++;
    }
  }

  // Print out info on the number of unique grains per variable vs the incoming bubble set sizes
  if (_t_step > _tracking_step)
  {
    bool display_them = false;
    for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
    {
      _console << "\nGrains active index " << map_num << ": " << map_sizes[map_num] << " -> " << _feature_counts_per_map[map_num];
      if (map_sizes[map_num] > _feature_counts_per_map[map_num])
      {
        _console << "--";
        display_them = true;
      }
      else if (map_sizes[map_num] < _feature_counts_per_map[map_num])
      {
        _console << "++";
        display_them = true;
      }
    }
    _console << std::endl;
  }

  /**
   * If it's the first time through this routine for the simulation, we will generate the unique grain
   * numbers using a simple counter.  These will be the unique grain numbers that we must track for
   * the remainder of the simulation.
   *
   * If we are linked to the EBSD Reader, we'll get the numbering information from its data
   * rather than generating our own.
   */
  if (_t_step == _tracking_step)   // Start tracking when the time_step == the tracking_step
  {
    if (_ebsd_reader)
    {
      auto grain_num = _ebsd_reader->getGrainNum();

      std::vector<Point> center_points(grain_num);

      for (decltype(grain_num) gr = 0; gr < grain_num; ++gr)
      {
        const EBSDReader::EBSDAvgData & d = _ebsd_reader->getAvgData(gr);
        center_points[gr] = d._p;

        Moose::out << "EBSD Grain " << gr << " " << center_points[gr] << '\n';
      }

      // To find the minimum distance we will use the centroidRegionDistance routine.
      std::vector<MeshTools::BoundingBox> ebsd_vector(1);

      std::set<unsigned int> used_indices;
      std::map<unsigned int, unsigned int> error_indices;

      if (grain_num != _feature_count && processor_id() == 0)
        mooseWarning("Mismatch:\nEBSD centers: " << grain_num << " Grain Tracker Centers: " << _feature_count);

      auto next_index = grain_num;

      // Loop over all of the features (grains)
      for (auto && feature : _feature_sets)
      {
        Real min_centroid_diff = std::numeric_limits<Real>::max();
        unsigned int closest_match_idx = 0;

        for (size_t j = 0; j < center_points.size(); ++j)
        {
          // Update the ebsd bbox data to be used in the centroidRegionDistance calculation
          // Since we are using centroid matching we'll just make it easy and set both the min/max of the box to the same
          // value (i.e. a zero sized box).
          ebsd_vector[0].min() = ebsd_vector[0].max() = center_points[j];

          Real curr_centroid_diff = centroidRegionDistance(ebsd_vector, feature._bboxes);
          if (curr_centroid_diff <= min_centroid_diff)
          {
            closest_match_idx = j;
            min_centroid_diff = curr_centroid_diff;
          }
        }

        if (used_indices.find(closest_match_idx) != used_indices.end())
        {
          Moose::out << "Re-assigning center " << closest_match_idx << " -> " << next_index << " "
                     << center_points[closest_match_idx] << " absolute distance: " << min_centroid_diff << '\n';
          feature._status = Status::MARKED;
          _unique_grains[next_index] = std::move(feature);

          _unique_grain_to_ebsd_num[next_index] = closest_match_idx;

          ++next_index;
        }
        else
        {
          Moose::out << "Assigning center " << closest_match_idx << " "
                     << center_points[closest_match_idx] << " absolute distance: " << min_centroid_diff << '\n';
          feature._status = Status::MARKED;
          _unique_grains[closest_match_idx] = std::move(feature);

          _unique_grain_to_ebsd_num[closest_match_idx] = closest_match_idx;

          used_indices.insert(closest_match_idx);
        }
      }

      if (!error_indices.empty())
      {
        for (const auto & grain_pair : _unique_grains)
          Moose::out << "Grain " << grain_pair.first << ": " << center_points[grain_pair.first] << '\n';

        Moose::out << "Error Indices:\n";
        for (const auto & error_kv : error_indices)
          Moose::out << "Grain " << error_kv.first << '(' << error_kv.second << ')' << ": " << center_points[error_kv.second] << '\n';

        mooseError("Error with ESBD Mapping (see above unused indices)");
      }
    }
    else
    {
      unsigned int counter = 0;
      // Move the grains from the FeatureFloodCount data structure to the _unique_grains data structure.
      for (auto && grain : _feature_sets)
      {
        _unique_grains.emplace_hint(_unique_grains.end(), std::pair<unsigned int, FeatureData>(counter, std::move(grain)));
        newGrainCreated(counter++);
      }

      // Clean up the "moved" Features
      _feature_sets.clear();
    }
    // Reserve op grain ids if we are using reserve_op. We'll mark the first index of the reserved id.
    // The remaining ids (if any) are sequential
    _reserve_grain_first_idx = _unique_grains.size();

    return;  // Return early - no matching or tracking to do
  }

  /**
   * To track grains across time steps, we will loop over our unique grains and link each one up with one of our new
   * unique grains.  The criteria for doing this will be to find the unique grain in the new list with a matching variable
   * index whose centroid is closest to this unique grain.
   */
  std::map<unsigned int, unsigned int> new_grain_idx_to_existing_grain_idx;

  for (auto & grain_pair : _unique_grains)
  {
    if (grain_pair.second._status == Status::INACTIVE)      // Don't try to find matches for inactive grains
      continue;

    unsigned int closest_match_idx;
    bool found_one = false;
    Real min_centroid_diff = std::numeric_limits<Real>::max();

    // We only need to examine grains that have matching variable indices
    for (size_t new_grain_idx = 0; new_grain_idx < _feature_sets.size(); ++new_grain_idx)
    {
      // TODO: It's possible to loop over just a subset of these indicies for efficiency
      if (grain_pair.second._var_idx == _feature_sets[new_grain_idx]._var_idx)  // Do the variables indicies match?
      {
        Real curr_centroid_diff = centroidRegionDistance(grain_pair.second._bboxes, _feature_sets[new_grain_idx]._bboxes);
        if (curr_centroid_diff <= min_centroid_diff)
        {
          found_one = true;
          closest_match_idx = new_grain_idx;
          min_centroid_diff = curr_centroid_diff;
        }
      }
    }

    if (found_one)
    {
      // Keep track of which new grains the existing ones want to map to
//      const auto match_pair = std::make_pair(map_idx, closest_match_idx);

     /**
      * It's possible that multiple existing grains will map to a single new grain (indicated by finding multiple
      * matches when we are building this map). This will happen any time a grain disappears during
      * this time step. We need to figure out the rightful owner in this case and inactivate the old grain.
      */
      const auto map_it = new_grain_idx_to_existing_grain_idx.find(closest_match_idx);

      if (map_it != new_grain_idx_to_existing_grain_idx.end())
      {
        // The new feature being competed for
        auto & feature = _feature_sets[closest_match_idx];

        // The two older grains competing (iterators into the map)
        const auto & grain_it1 = _unique_grains.find(map_it->second);
        const auto & grain_it2 = _unique_grains.find(grain_pair.first);

        auto centroid_diff1 = centroidRegionDistance(feature._bboxes, grain_it1->second._bboxes);
        auto centroid_diff2 = centroidRegionDistance(feature._bboxes, grain_it2->second._bboxes);

        auto & inactive_it = (centroid_diff1 < centroid_diff2) ? grain_it2 : grain_it1;

        inactive_it->second._status = Status::INACTIVE;
        _console << "Marking Grain " << inactive_it->first << " as INACTIVE (variable index: "
                 << inactive_it->second._var_idx << ")\n"
                 << inactive_it->second;

        // Make sure we update the new to existing map if necessary
        if (grain_it1->first == inactive_it->first)
          new_grain_idx_to_existing_grain_idx[closest_match_idx] = grain_pair.first;
      }
      else
        new_grain_idx_to_existing_grain_idx[closest_match_idx] = grain_pair.first;
    }
  }

  // Transfer ownership of all grains where we found a match
  for (const auto & new_to_exist_kv : new_grain_idx_to_existing_grain_idx)
  {
    auto curr_idx = new_to_exist_kv.second;
                                                       // feature index
    _unique_grains[curr_idx] = std::move(_feature_sets[new_to_exist_kv.first]);
    _unique_grains[curr_idx]._status = Status::MARKED;
  }

  //  Next we need to look at our new list and see which grains weren't matched up.  These are new grains.
  for (auto feature_num = decltype(_feature_sets.size())(0); feature_num < _feature_sets.size(); ++feature_num)
    // If it's not in the index list, it hasn't been transferred
    if (new_grain_idx_to_existing_grain_idx.find(feature_num) == new_grain_idx_to_existing_grain_idx.end())
    {
      mooseAssert(_feature_sets[feature_num]._status == Status::CLEAR, "Feature in wrong state, logic error");

      auto new_idx = _unique_grains.size() + _n_reserve_ops;

      _feature_sets[feature_num]._status = Status::MARKED;               // Mark it
      _unique_grains[new_idx] = std::move(_feature_sets[feature_num]);   // transfer ownership

      // Save off the ids of the newly created grains for the remaining ranks
      new_grain_indices.emplace_back(new_idx);
    }

  /**
   * Finally we need to mark any grains in the unique list that aren't marked as inactive.  These are the unique grains
   * that didn't match up to any new feature. This should only happen if it's the last active grain for
   * this particular variable.
   */
  for (auto & grain_pair : _unique_grains)
    if (grain_pair.second._status == Status::CLEAR)
    {
      grain_pair.second._status = Status::INACTIVE;
      _console << "Marking Grain " << grain_pair.first << " as INACTIVE (variable index: "
               << grain_pair.second._var_idx <<  ")\n"
               << grain_pair.second;
    }
}

void
GrainTracker::newGrainCreated(unsigned int new_grain_idx)
{
  if (_t_step > _tracking_step)
    _console << COLOR_YELLOW
             << "*****************************************************************************\n"
             << "Couldn't find a matching grain while working on variable index: " << _unique_grains[new_grain_idx]._var_idx
             << "\nCreating new unique grain: " << new_grain_idx << '\n' << _unique_grains[new_grain_idx]
             << "\n*****************************************************************************\n" << COLOR_DEFAULT;
}

void
GrainTracker::buildLocalToGlobalIndices(std::vector<std::vector<unsigned int> > & local_to_global_all) const
{
  mooseAssert(processor_id() == 0, "This method must only be called on the root processor");

  local_to_global_all.resize(_n_procs, std::vector<unsigned int>(_max_local_size));

  for (const auto & grain_pair : _unique_grains)
  {
    // Get the local indicies from the feature and build a map
    for (const auto & local_index_pair : grain_pair.second._orig_ids)
    {
      mooseAssert(local_index_pair.first < _n_procs, local_index_pair.first << ", " << _n_procs);
      mooseAssert(local_index_pair.second < _max_local_size, local_index_pair.second << ", " << _max_local_size);

      std::cout << local_index_pair.first << " : " << local_index_pair.second << " : " << grain_pair.first << '\n';


                              // rank                 // local index
      local_to_global_all[local_index_pair.first][local_index_pair.second] = grain_pair.first;
    }
  }
}

void
GrainTracker::remapGrains()
{
  // Don't remap grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  _console << "Running remap Grains" << std::endl;

  /**
   * The remapping algorithm is recursive. We will use the status variable in each FeatureData
   * to track which grains are currently being remapped so we don't have runaway recursion.
   * To begin we need to reset all of the active (MARKED) flags to NOT_MARKED.
   *
   * Additionally we need to record each grain's variable index so that we can communicate
   * changes to the non-root ranks later in a single batch.
   */
  std::vector<unsigned int> grain_id_to_existing_var_idx(_unique_grains.size(), std::numeric_limits<unsigned int>::max());
  for (auto & grain_pair : _unique_grains)
  {
    mooseAssert(!(grain_pair.second._status == Status::CLEAR), "Grain " << grain_pair.first << " status in wrong state.");

    if (grain_pair.second._status == Status::MARKED)
    {
      grain_pair.second._status = Status::CLEAR;
      grain_id_to_existing_var_idx[grain_pair.first] = grain_pair.second._var_idx;
    }
  }

  /**
   * Loop over each grain and see if any grains represented by the same variable are "touching"
   */
  bool any_grains_remapped = false;
  bool grains_remapped;
  do
  {
    grains_remapped = false;
    for (auto grain_it1 = _unique_grains.begin(); grain_it1 != _unique_grains.end(); ++grain_it1)
    {
      if (grain_it1->second._status == Status::INACTIVE)
        continue;

      // We need to remap any grains represented on any variable index above the cuttoff
      if (grain_it1->second._var_idx >= _reserve_op_idx)
      {
        Moose::out
          << COLOR_YELLOW
          << "Grain #" << grain_it1->first << " detected on a reserved order parameter #" << grain_it1->second._var_idx << ", remapping to another variable\n"
          << COLOR_DEFAULT;

        for (auto max = decltype(_max_renumbering_recursion)(0); max <= _max_renumbering_recursion; ++max)
          if (max < _max_renumbering_recursion)
          {
            if (attemptGrainRenumber(grain_it1->second, grain_it1->first, 0, max))
              break;
          }
          else if (!attemptGrainRenumber(grain_it1->second, grain_it1->first, 0, max))
            mooseError(COLOR_RED << "Unable to find any suitable order parameters for remapping. Perhaps you need more op variables?\n\n" << COLOR_DEFAULT);

        grains_remapped = true;
      }

      for (auto grain_it2 = _unique_grains.begin(); grain_it2 != _unique_grains.end(); ++grain_it2)
      {
        // Don't compare a grain with itself and don't try to remap inactive grains
        if (grain_it1 == grain_it2 || grain_it2->second._status == Status::INACTIVE)
          continue;

        if (grain_it1->second._var_idx == grain_it2->second._var_idx &&     // Are the grains represented by the same variable?
            grain_it1->second.boundingBoxesIntersect(grain_it2->second) &&  // If so, do their bboxes intersect (coarse level check)?
            grain_it1->second.halosIntersect(grain_it2->second))            // If so, do they actually overlap (tight "hull" check)?
        {
          Moose::out
            << COLOR_YELLOW
            << "Grain #" << grain_it1->first << " intersects Grain #" << grain_it2->first
            << " (variable index: " << grain_it1->second._var_idx << ")\n"
            << COLOR_DEFAULT;

          for (auto max = decltype(_max_renumbering_recursion)(0); max <= _max_renumbering_recursion; ++max)
            if (max < _max_renumbering_recursion)
            {
              if (attemptGrainRenumber(grain_it1->second, grain_it1->first, 0, max) || attemptGrainRenumber(grain_it2->second, grain_it2->first, 0, max))
                break;
            }
            else if (!attemptGrainRenumber(grain_it1->second, grain_it1->first, 0, max) && !attemptGrainRenumber(grain_it2->second, grain_it2->first, 0, max))
              mooseError(COLOR_RED << "Unable to find any suitable order parameters for remapping. Perhaps you need more op variables?\n\n" << COLOR_DEFAULT);

          grains_remapped = true;
        }
      }
    }
    any_grains_remapped |= grains_remapped;
  }
  while (grains_remapped);

  /**
   * Now actually preform the remapping
   */
  std::cout << "\n\nStarting Actual Remapping\n";

  if (any_grains_remapped)
  {
    std::vector<std::map<Node *, CacheValues> > cache(_n_vars);
    for (auto & grain_pair : _unique_grains)
    {
      auto & grain = grain_pair.second;
      auto old_var_idx = grain_id_to_existing_var_idx[grain_pair.first];

      if (grain._status != Status::INACTIVE && old_var_idx != grain._var_idx)
      {
        std::cout << "Caching solution values on grain #" << grain_pair.first << " from variable index " << old_var_idx << '\n';

        mooseAssert(old_var_idx != std::numeric_limits<unsigned int>::max(), "old_var_idx in incorrect state");
        swapSolutionValues(grain, old_var_idx, cache, RemapCacheMode::FILL);
      }
    }

    for (auto & grain_pair : _unique_grains)
    {
      auto & grain = grain_pair.second;
      auto old_var_idx = grain_id_to_existing_var_idx[grain_pair.first];

      if (grain._status != Status::INACTIVE && old_var_idx != grain._var_idx)
      {
        std::cout << "Writing solution values from cache on grain #" << grain_pair.first << " to variable index " << grain._var_idx << '\n';

        mooseAssert(old_var_idx != std::numeric_limits<unsigned int>::max(), "old_var_idx in incorrect state");
        swapSolutionValues(grain, old_var_idx, cache, RemapCacheMode::USE);
      }
    }

    _nl.solution().close();
    _nl.solutionOld().close();
    _nl.solutionOlder().close();

    _fe_problem.getNonlinearSystem().sys().update();
  }
}

void
GrainTracker::computeMinDistancesFromGrain(FeatureData & grain,
                                           std::vector<std::list<GrainDistance> > & min_distances)
{
  /**
   *  In the diagram below assume we have 4 order parameters. The grain with the asterisk needs to be
   *  remapped. All order parameters are used in neighboring grains. For all "touching" grains, the value
   *  of the corresponding entry in min_distances will be a negative integer representing the number of
   *  immediate neighbors with that order parameter.
   *  Note: Only the first member of the pair (the distance) is shown in the array below.
   *
   *  e.g. [-2.0, -max, -1.0, -2.0]
   *
   *  After sorting, variable index 2 (value: -1.0) be at the end of the array and will be the first variable
   *  we attempt to renumber the current grain to.
   *
   *        __       ___
   *          \  0  /   \
   *        2  \___/  1  \___
   *           /   \     /   \
   *        __/  1  \___/  2  \
   *          \  *  /   \     /
   *        3  \___/  3  \___/
   *           /   \     /
   *        __/  0  \___/
   *
   */
  for (auto & grain_pair : _unique_grains)
  {
    if (grain_pair.second._status == Status::INACTIVE || grain_pair.second._var_idx == grain._var_idx ||
        (grain_pair.second._var_idx >= _reserve_op_idx))
      continue;

    unsigned int target_var_index = grain_pair.second._var_idx;
    unsigned int target_grain_id = grain_pair.first;

    Real curr_sphere_diff = boundingRegionDistance(grain._bboxes, grain_pair.second._bboxes);

    GrainDistance grain_distance_obj(curr_sphere_diff, target_grain_id, target_var_index);

    // To handle touching halos we penalize the top pick each time we see another
    if (curr_sphere_diff == -1.0 && !min_distances[target_var_index].empty())
    {
      Real last_distance = min_distances[target_var_index].begin()->_distance;
      if (last_distance < 0)
        grain_distance_obj._distance += last_distance;
    }

    // Insertion sort into a list
    auto insert_it = min_distances[target_var_index].begin();
    while (insert_it != min_distances[target_var_index].end() && !(grain_distance_obj < *insert_it))
      ++insert_it;
    min_distances[target_var_index].insert(insert_it, grain_distance_obj);
  }
}

bool
GrainTracker::attemptGrainRenumber(FeatureData & grain, unsigned int grain_id, unsigned int depth, unsigned int max)
{
  // End the recursion of our breadth first search
  if (depth > max)
    return false;

  unsigned int curr_var_idx = grain._var_idx;

  std::vector<std::map<Node *, CacheValues> > cache;

  std::vector<std::list<GrainDistance> > min_distances(_vars.size());

  /**
   * We have two grains that are getting close represented by the same order parameter.
   * We need to map to the variable whose closest grain to this one is furthest away by sphere to sphere distance.
   */
  computeMinDistancesFromGrain(grain, min_distances);

  /**
   * We have a vector of the distances to the closest grains represented by each of our variables.  We just need to pick
   * a suitable grain to replace with.  We will start with the maximum of this this list: (max of the mins), but will settle
   * for next to largest and so forth as we make more attempts at remapping grains.  This is a graph coloring problem so
   * more work will be required to optimize this process.
   *
   * Note: We don't have an explicit check here to avoid remapping a  variable to itself.  This is unnecessary since the
   * min_distance of a variable is explicitly set up above.
   */

  std::sort(min_distances.begin(), min_distances.end(),
            [](const std::list<GrainDistance> & lhs, const std::list<GrainDistance> & rhs)
            {
              // Sort lists in reverse order (largest distance first)
              // These empty cases are here to make this comparison stable
              if (lhs.empty())
                return false;
              else if (rhs.empty())
                return true;
              else
                return lhs.begin()->_distance > rhs.begin()->_distance;
            });

  for (auto & list_ref : min_distances)
  {
    const auto target_it = list_ref.begin();
    if (target_it == list_ref.end())
      continue;

    // If the distance is positive we can just remap and be done
    if (target_it->_distance > 0)
    {
      Moose::out
        << COLOR_GREEN
        << "- Depth " << depth << ": Remapping grain #" << grain_id << " from variable index " << curr_var_idx
        << " to " << target_it->_var_index << " whose closest grain (#" << target_it->_grain_id
        << ") is at a distance of " << target_it->_distance << "\n"
        << COLOR_DEFAULT;

      grain._status |= Status::DIRTY;
      grain._var_idx = target_it->_var_index;
      return true;
    }

    // If the distance isn't positive we just need to make sure that none of the grains represented by the
    // target variable index would intersect this one if we were to remap
    auto next_target_it = target_it;
    bool intersection_hit = false;
    std::ostringstream oss;
    while (!intersection_hit && next_target_it != list_ref.end())
    {
      if (next_target_it->_distance > 0)
        break;

      mooseAssert(_unique_grains.find(next_target_it->_grain_id) != _unique_grains.end(), "Error in indexing target grain in attemptGrainRenumber");
      FeatureData & next_target_grain = _unique_grains[next_target_it->_grain_id];

      // If any grains touch we're done here
      if (grain.halosIntersect(next_target_grain))
        intersection_hit = true;
      else
        oss << " #" << next_target_it->_grain_id;

      ++next_target_it;
    }

    if (!intersection_hit)
    {
      Moose::out
        << COLOR_GREEN
        << "- Depth " << depth << ": Remapping grain #" << grain_id << " from variable index " << curr_var_idx
        << " to " << target_it->_var_index << " whose closest grain(s):" << oss.str()
        << " are inside our bounding sphere but whose halo(s) are not touching.\n"
        << COLOR_DEFAULT;

      grain._status |= Status::DIRTY;
      grain._var_idx = target_it->_var_index;
      return true;
    }

    // If we reach this part of the loop, there is no simple renumbering that can be done.
    mooseAssert(_unique_grains.find(target_it->_grain_id) != _unique_grains.end(), "Error in indexing target grain in attemptGrainRenumber");
    FeatureData & target_grain = _unique_grains[target_it->_grain_id];

    /**
     * If we get to this case and the best distance is less than -1, we are in big trouble. This means that grains represented by all of
     * the remaining order parameters are overlapping this one in at least two places. We'd have to maintain multiple recursive chains,
     * or just start over from scratch...
     * Let's just return false and see if there is another remapping option.
     */
    if (target_it->_distance < -1)
      return false;

    // Make sure this grain isn't marked. If it is, we can't recurse here
    if ((target_grain._status & Status::MARKED) == Status::MARKED)
      return false;

    /**
     * Propose a new variable index for the current grain and recurse.
     * We don't need to mark the status as DIRTY here since the recursion
     * may fail. For now, we'll just add MARKED to the status.
     */
    grain._var_idx = target_it->_var_index;
    grain._status |= Status::MARKED;
    if (attemptGrainRenumber(target_grain, target_it->_grain_id, depth+1, max))
    {
      // SUCCESS!
      Moose::out
        << COLOR_GREEN
        << "- Depth " << depth << ": Remapping grain #" << grain_id << " from variable index " << curr_var_idx
        << " to " << target_it->_var_index << '\n'
        << COLOR_DEFAULT;

      // Now we need to mark the grain as DIRTY since the recursion succeeded.
      grain._status |= Status::DIRTY;
      return true;
    }
    else
      // FAILURE, We need to set our var index back after failed recursive step
      grain._var_idx = curr_var_idx;

    // ALWAYS "unmark" (or clear the MARKED status) after recursion so it can be used by other remap operations
    grain._status &= ~Status::MARKED;
  }

  return false;
}

void
GrainTracker::swapSolutionValues(FeatureData & grain, unsigned int old_var_idx, std::vector<std::map<Node *, CacheValues> > & cache,
                                 RemapCacheMode cache_mode)
{
  MeshBase & mesh = _mesh.getMesh();

  // Remap the grain
  std::set<Node *> updated_nodes_tmp; // Used only in the elemental case
  for (auto entity : grain._local_ids)
  {
    if (_is_elemental)
    {
      Elem * elem = mesh.query_elem(entity);
      if (!elem)
        continue;

      for (unsigned int i = 0; i < elem->n_nodes(); ++i)
      {
        Node * curr_node = elem->get_node(i);
        if (updated_nodes_tmp.find(curr_node) == updated_nodes_tmp.end())
        {
          updated_nodes_tmp.insert(curr_node); // cache this node so we don't attempt to remap it again within this loop
          swapSolutionValuesHelper(curr_node, grain._var_idx, old_var_idx, cache, cache_mode);
        }
      }
    }
    else
      swapSolutionValuesHelper(mesh.query_node_ptr(entity), grain._var_idx, old_var_idx, cache, cache_mode);
  }
//  // Update the variable index in the unique grain datastructure
//  grain._var_idx = var_idx;
}

void
GrainTracker::swapSolutionValuesHelper(Node * curr_node, unsigned int curr_var_idx, unsigned int old_var_idx,
                                       std::vector<std::map<Node *, CacheValues> > & cache, RemapCacheMode cache_mode)
{
  if (curr_node && curr_node->processor_id() == processor_id())
  {
    // Reinit the node so we can get and set values of the solution here
    _subproblem.reinitNode(curr_node, 0);

    // Local variables to hold values being transferred
    Real current, old, older;

    // Retrieve the value either from the old variable or cache
    if (cache_mode == RemapCacheMode::FILL || cache_mode == RemapCacheMode::BYPASS)
    {
      current = _vars[old_var_idx]->nodalSln()[0];
      old = _vars[old_var_idx]->nodalSlnOld()[0];
      older = _vars[old_var_idx]->nodalSlnOlder()[0];
    }
    else // USE
    {
      const auto cache_it = cache[old_var_idx].find(curr_node);
      mooseAssert(cache_it != cache[old_var_idx].end(), "Error in cache");
      current = cache_it->second.current;
      old = cache_it->second.old;
      older = cache_it->second.older;
    }

    // Cache the value or use it!
    if (cache_mode == RemapCacheMode::FILL)
    {
      cache[old_var_idx][curr_node].current = current;
      cache[old_var_idx][curr_node].old = old;
      cache[old_var_idx][curr_node].older = older;
    }
    else // USE or BYPASS
    {
      const auto & dof_index = _vars[curr_var_idx]->nodalDofIndex();

      // Transfer this solution from the old to the current
      _nl.solution().set(dof_index, current);
      _nl.solutionOld().set(dof_index, old);
      _nl.solutionOlder().set(dof_index, older);
    }

    /**
     * Finally zero out the old variable. When using the FILL/USE combination to
     * read/write variables, it's important to zero the variable on the FILL
     * stage and not the USE stage. The reason for this is handling swaps as
     * illustrated in the following diagram
     *       ___  ___
     *      /   \/   \    If adjacent grains (overlapping flood region) end up
     *     /  1 /\ 2  \   swapping variable indices and variables are zeroed on
     *     \  2*\/ 1* /   "USE", the overlap region will be incorrectly zeroed
     *      \___/\___/    by whichever variable is written to second.
     *.
     */
    if (cache_mode == RemapCacheMode::FILL || cache_mode == RemapCacheMode::BYPASS)
    {
      const auto & dof_index = _vars[old_var_idx]->nodalDofIndex();

      // Set the DOF for the current variable to zero
      _nl.solution().set(dof_index, 0.0);
      _nl.solutionOld().set(dof_index, 0.0);
      _nl.solutionOlder().set(dof_index, 0.0);
    }
  }
}

void
GrainTracker::updateFieldInfo()
{
  // Whether or not we should store and write out volume information
  bool gather_volumes = _pars.isParamValid("bubble_volume_file");
  if (gather_volumes)
  {
    // store volumes per feature
    _all_feature_volumes.reserve(_unique_grains.size());

    // store totals per variable (or smaller)
    _total_volume_intersecting_boundary.resize(_single_map_mode || _condense_map_info ? 0 : _maps_size);
  }

  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
    _feature_maps[map_num].clear();

  std::map<unsigned int, Real> tmp_map;
  MeshBase & mesh = _mesh.getMesh();

  for (const auto & grain_pair : _unique_grains)
  {
    unsigned int curr_var = grain_pair.second._var_idx;
    unsigned int map_idx = (_single_map_mode || _condense_map_info) ? 0 : curr_var;

    if (grain_pair.second._status == Status::INACTIVE)
      continue;

    for (auto entity : grain_pair.second._local_ids)
    {
      // Highest variable value at this entity wins
      Real entity_value = -std::numeric_limits<Real>::max();
      if (_is_elemental)
      {
        const Elem * elem = mesh.elem(entity);
        std::vector<Point> centroid(1, elem->centroid());
        _fe_problem.reinitElemPhys(elem, centroid, 0);
        entity_value = _vars[curr_var]->sln()[0];
      }
      else
      {
        Node & node = mesh.node(entity);
        entity_value = _vars[curr_var]->getNodalValue(node);
      }

      if (tmp_map.find(entity) == tmp_map.end() || entity_value > tmp_map[entity])
      {
        // TODO: Add an option for EBSD Reader
        _feature_maps[map_idx][entity] = _ebsd_reader ? _unique_grain_to_ebsd_num[grain_pair.first] : grain_pair.first;
        if (_var_index_mode)
          _var_index_maps[map_idx][entity] = grain_pair.second._var_idx;

        tmp_map[entity] = entity_value;
      }
    }
    for (auto entity : grain_pair.second._halo_ids)
      _halo_ids[grain_pair.second._var_idx][entity] += grain_pair.second._var_idx;

    for (auto entity : grain_pair.second._ghosted_ids)
      _ghosted_entity_ids[entity] = 1;

    // Save off volume information (no sort required)
    if (gather_volumes)
    {
      _all_feature_volumes.push_back(grain_pair.second._volume);
      if (grain_pair.second._intersects_boundary)
        _total_volume_intersecting_boundary[map_idx] += grain_pair.second._volume;
    }
  }
}

Real
GrainTracker::centroidRegionDistance(std::vector<MeshTools::BoundingBox> & bboxes1, std::vector<MeshTools::BoundingBox> bboxes2) const
{
  /**
   * Find the minimum centroid distance between any to pieces of the grains.
   */
  auto min_distance = std::numeric_limits<Real>::max();
  for (const auto & bbox1 : bboxes1)
  {
    const auto centroid_point1 = (bbox1.max() + bbox1.min()) / 2.0;

    for (const auto & bbox2 : bboxes2)
    {
      const auto centroid_point2 = (bbox2.max() + bbox2.min()) / 2.0;

      // Here we'll calculate a distance between the centroids
      auto curr_distance = _mesh.minPeriodicDistance(_var_number, centroid_point1, centroid_point2);

      if (curr_distance < min_distance)
        min_distance = curr_distance;
    }
  }

  return min_distance;
}

Real
GrainTracker::boundingRegionDistance(std::vector<MeshTools::BoundingBox> & bboxes1, std::vector<MeshTools::BoundingBox> bboxes2) const
{
  /**
   * The region that each grain covers is represented by a bounding box large enough to encompassing all the points
   * within that grain. When using periodic boundaries, we may have several discrete "pieces" of a grain each represented
   * by a bounding box. The distance between any two grains is defined as the minimum distance between any pair of boxes,
   * one selected from each grain.
   */
  auto min_distance = std::numeric_limits<Real>::max();
  for (const auto & bbox1 : bboxes1)
  {
    for (const auto & bbox2 : bboxes2)
    {
      // AABB squared distance
      Real curr_distance = 0.0;
      bool boxes_overlap = true;
      for (unsigned int dim = 0; dim < LIBMESH_DIM; ++dim)
      {
        const auto & min1 = bbox1.min()(dim);
        const auto & max1 = bbox1.max()(dim);
        const auto & min2 = bbox2.min()(dim);
        const auto & max2 = bbox2.max()(dim);

        if (min1 > max2)
        {
          const auto delta = max2 - min1;
          curr_distance += delta * delta;
          boxes_overlap = false;
        }
        else if (min2 > max1)
        {
          const auto delta = max1 - min2;
          curr_distance += delta * delta;
          boxes_overlap = false;
        }
      }

      if (boxes_overlap)
        return -1.0; /* all overlaps are treated the same */

      if (curr_distance < min_distance)
        min_distance = curr_distance;
    }
  }

  return min_distance;
}

/*************************************************
 ************** Helper Structures ****************
 ************************************************/
GrainDistance::GrainDistance() :
    _distance(std::numeric_limits<Real>::max()),
    _grain_id(std::numeric_limits<unsigned int>::max()),
    _var_index(std::numeric_limits<unsigned int>::max())
{
}

GrainDistance::GrainDistance(Real distance, unsigned int grain_id, unsigned int var_index) :
    _distance(distance),
    _grain_id(grain_id),
    _var_index(var_index)
{
}

bool
GrainDistance::operator<(const GrainDistance & rhs) const
{
  return _distance < rhs._distance;
}

std::vector<unsigned int> GrainTracker::_empty_2;
