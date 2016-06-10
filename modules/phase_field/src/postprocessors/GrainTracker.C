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
    _remap(getParam<bool>("remap_grains")),
    _nl(static_cast<FEProblem &>(_subproblem).getNonlinearSystem()),
    _unique_grains(declareRestartableData<std::map<unsigned int, FeatureData> >("unique_grains")),
    _ebsd_reader(parameters.isParamValid("ebsd_reader") ? &getUserObject<EBSDReader>("ebsd_reader") : nullptr),
    _compute_op_maps(getParam<bool>("compute_op_maps"))
{
  if (!_is_elemental && _compute_op_maps)
    mooseError("\"compute_op_maps\" is only supported with \"flood_entity_type = ELEMENTAL\"");
}

GrainTracker::~GrainTracker()
{
}

Real
GrainTracker::getEntityValue(dof_id_type node_id, FIELD_TYPE field_type, unsigned int var_idx) const
{
  if (_t_step < _tracking_step)
    return 0;

  return FeatureFloodCount::getEntityValue(node_id, field_type, var_idx);
}

void
GrainTracker::initialize()
{
  FeatureFloodCount::initialize();

  _elemental_data.clear();
}

void
GrainTracker::execute()
{
  Moose::perf_log.push("execute()", "GrainTracker");
  FeatureFloodCount::execute();
  Moose::perf_log.pop("execute()", "GrainTracker");
}

void
GrainTracker::finalize()
{
  Moose::perf_log.push("finalize()", "GrainTracker");

  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  expandHalos();

  FeatureFloodCount::communicateAndMerge();

  _console << "Finished inside of FeatureFloodCount" << std::endl;

  Moose::perf_log.push("trackGrains()","GrainTracker");
  trackGrains();
  Moose::perf_log.pop("trackGrains()","GrainTracker");

  _console << "Finished inside of trackGrains" << std::endl;

  Moose::perf_log.push("remapGrains()","GrainTracker");
  if (_remap)
    remapGrains();
  Moose::perf_log.pop("remapGrains()","GrainTracker");

  updateFieldInfo();

  _console << "Finished inside of updateFieldInfo" << std::endl;

  // Calculate and out output bubble volume data
  if (_pars.isParamValid("bubble_volume_file"))
  {
    calculateBubbleVolumes();
    std::vector<Real> data; data.reserve(_all_feature_volumes.size() + 2);
    data.push_back(_fe_problem.timeStep());
    data.push_back(_fe_problem.time());
    data.insert(data.end(), _all_feature_volumes.begin(), _all_feature_volumes.end());
    writeCSVFile(getParam<FileName>("bubble_volume_file"), data);
  }

  if (_compute_op_maps)
  {
    for (const auto & grain_pair : _unique_grains)
    {
      if (grain_pair.second._status != INACTIVE)
      {
        for (auto elem_id : grain_pair.second._local_ids)
        {
          mooseAssert(!_ebsd_reader || _unique_grain_to_ebsd_num.find(grain_pair.first) != _unique_grain_to_ebsd_num.end(), "Bad mapping in unique_grain_to_ebsd_num");
          _elemental_data[elem_id].push_back(std::make_pair(_ebsd_reader ? _unique_grain_to_ebsd_num[grain_pair.first] : grain_pair.first, grain_pair.second._var_idx));
        }
      }
    }
  }

  Moose::perf_log.pop("finalize()", "GrainTracker");
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

void
GrainTracker::expandHalos()
{
  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
    for (auto rank = decltype(_n_procs)(0); rank < _n_procs; ++rank)
      for (auto & feature : _partial_feature_sets[rank][map_num])
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
              visitElementalNeighbors(_mesh.elemPtr(entity), feature._var_idx, &feature, /*recurse =*/false);
            else
              visitNodalNeighbors(_mesh.nodePtr(entity), feature._var_idx, &feature, /*recurse =*/false);
          }
        }
      }
}

void
GrainTracker::trackGrains()
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  // Reset Status on active unique grains
  std::vector<unsigned int> map_sizes(_maps_size);
  for (auto & grain_pair : _unique_grains)
  {
    if (grain_pair.second._status != INACTIVE)
    {
      grain_pair.second._status = NOT_MARKED;
      map_sizes[grain_pair.second._var_idx]++;
    }
  }

  // Print out info on the number of unique grains per variable vs the incoming bubble set sizes
  if (_t_step > _tracking_step)
  {
    bool display_them = false;
    for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
    {
      _console << "\nGrains active index " << map_num << ": " << map_sizes[map_num] << " -> " << _feature_sets[map_num].size();
      if (map_sizes[map_num] > _feature_sets[map_num].size())
      {
        _console << "--";
        display_them = true;
      }
      else if (map_sizes[map_num] < _feature_sets[map_num].size())
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

      for (unsigned int gr = 0; gr < grain_num; ++gr)
      {
        const EBSDReader::EBSDAvgData & d = _ebsd_reader->getAvgData(gr);
        center_points[gr] = d._p;

        Moose::out << "EBSD Grain " << gr << " " << center_points[gr] << '\n';
      }

      // To find the minimum distance we will use the centroidRegionDistance routine.
      std::vector<MeshTools::BoundingBox> ebsd_vector(1);

      std::set<unsigned int> used_indices;
      std::map<unsigned int, unsigned int> error_indices;

      unsigned int total_grains = 0;
      for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
        total_grains += _feature_sets[map_num].size();

      if (grain_num != total_grains && processor_id() == 0)
        mooseWarning("Mismatch:\nEBSD centers: " << grain_num << " Grain Tracker Centers: " << total_grains);

      auto next_index = grain_num;

      // Loop over all of the features (grains)
      for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
        for (unsigned int feature_num = 0; feature_num < _feature_sets[map_num].size(); ++feature_num)
        {
          Real min_centroid_diff = std::numeric_limits<Real>::max();
          unsigned int closest_match_idx = 0;

          for (unsigned int j = 0; j < center_points.size(); ++j)
          {
            // Update the ebsd bbox data to be used in the centroidRegionDistance calculation
            // Since we are using centroid matching we'll just make it easy and set both the min/max of the box to the same
            // value (i.e. a zero sized box).
            ebsd_vector[0].min() = ebsd_vector[0].max() = center_points[j];

            Real curr_centroid_diff = centroidRegionDistance(ebsd_vector, _feature_sets[map_num][feature_num]._bboxes);
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
            _unique_grains[next_index] = std::move(_feature_sets[map_num][feature_num]);

            _unique_grain_to_ebsd_num[next_index] = closest_match_idx;

            ++next_index;
          }
          else
          {
            Moose::out << "Assigning center " << closest_match_idx << " "
                       << center_points[closest_match_idx] << " absolute distance: " << min_centroid_diff << '\n';
            _unique_grains[closest_match_idx] = std::move(_feature_sets[map_num][feature_num]);

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
      /**
       * Here we want to assign the grains in some partitioning invariant way. We'll sort first by
       * _var_idx (already partitioned in that manner) then sort on the _min_entity_id
       * to order the grains.
       */
      unsigned int counter = 0;
      for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
      {
        // Sort the grains represented by this variable by _min_entity_id
        std::sort(_feature_sets[map_num].begin(), _feature_sets[map_num].end());

        /**
         * Transfer the grains to the _unique_grains structure and assign ids
         * Transform a vector that looks like this { A, D, C, B, F, E}
         * into a map like this { {1,A}, {2,D}, {3,D}, {4, B}, {5, F}, {6, E} }
         */
        std::transform(_feature_sets[map_num].begin(), _feature_sets[map_num].end(), std::inserter(_unique_grains, _unique_grains.end()),
                       [&counter](FeatureData & item)
                       {
                         return std::move(std::pair<unsigned int, FeatureData>(counter++, std::move(item)));
                       });
      }
    }
    return;  // Return early - no matching or tracking to do
  }

  /**
   * To track grains across time steps, we will loop over our unique grains and link each one up with one of our new
   * unique grains.  The criteria for doing this will be to find the unique grain in the new list with a matching variable
   * index whose centroid is closest to this unique grain.
   */
  std::map<std::pair<unsigned int, unsigned int>, std::vector<unsigned int> > new_grain_idx_to_existing_grain_idx;

  for (auto & grain_pair : _unique_grains)
  {
    if (grain_pair.second._status == INACTIVE)                         // Don't try to find matches for inactive grains
      continue;

    unsigned int closest_match_idx;
    bool found_one = false;
    Real min_centroid_diff = std::numeric_limits<Real>::max();

    // We only need to examine grains that have matching variable indices
    unsigned int map_idx = _single_map_mode ? 0 : grain_pair.second._var_idx;
    for (unsigned int new_grain_idx = 0; new_grain_idx < _feature_sets[map_idx].size(); ++new_grain_idx)
    {
      if (grain_pair.second._var_idx == _feature_sets[map_idx][new_grain_idx]._var_idx)  // Do the variables indicies match?
      {
        Real curr_centroid_diff = centroidRegionDistance(grain_pair.second._bboxes, _feature_sets[map_idx][new_grain_idx]._bboxes);
        if (curr_centroid_diff <= min_centroid_diff)
        {
          found_one = true;
          closest_match_idx = new_grain_idx;
          min_centroid_diff = curr_centroid_diff;
        }
      }
    }

    if (found_one)
      // Keep track of which new grains the existing ones want to map to
      new_grain_idx_to_existing_grain_idx[std::make_pair(map_idx, closest_match_idx)].push_back(grain_pair.first);
  }

  /**
   * It's possible that multiple existing grains will map to a single new grain (indicated by multiplicity in the
   * new_grain_idx_to_existing_grain_idx data structure).  This will happen any time a grain disappears during
   * this time step. We need to figure out the rightful owner in this case and inactivate the old grain.
   */
  for (const auto & new_to_exist_kv : new_grain_idx_to_existing_grain_idx)
  {                                                             // map index     feature index
    FeatureData feature = std::move(_feature_sets[new_to_exist_kv.first.first][new_to_exist_kv.first.second]);

    // If there is only a single mapping - we've found the correct grain
    if (new_to_exist_kv.second.size() == 1)
    {
      unsigned int curr_idx = (new_to_exist_kv.second)[0];
      feature._status = MARKED;                             // Mark it
      _unique_grains[curr_idx] = std::move(feature);        // transfer ownership of new grain
    }

    // More than one existing grain is mapping to a new one (i.e. multiple values exist for a single key)
    else
    {
      Real min_centroid_diff = std::numeric_limits<Real>::max();
      unsigned int min_idx = 0;

      for (unsigned int i = 0; i < new_to_exist_kv.second.size(); ++i)
      {
        unsigned int curr_idx = (new_to_exist_kv.second)[i];

        Real curr_centroid_diff = centroidRegionDistance(feature._bboxes, _unique_grains[curr_idx]._bboxes);
        if (curr_centroid_diff <= min_centroid_diff)
        {
          min_idx = i;
          min_centroid_diff = curr_centroid_diff;
        }
      }

      // One more time over the competing indices.  We will mark the non-winners as inactive and transfer ownership to the winner (the closest centroid).
      for (unsigned int i = 0; i < new_to_exist_kv.second.size(); ++i)
      {
        unsigned int curr_idx = (new_to_exist_kv.second)[i];
        if (i == min_idx)
        {
          feature._status = MARKED;                          // Mark it
          _unique_grains[curr_idx] = std::move(feature);     // transfer ownership of new grain
        }
        else
        {
          _console << "Marking Grain " << curr_idx << " as INACTIVE (variable index: "
                   << _unique_grains[curr_idx]._var_idx << ")\n"
                   << _unique_grains[curr_idx];
          _unique_grains[curr_idx]._status = INACTIVE;
        }
      }
    }
  }

  /**
   * Next we need to look at our new list and see which grains weren't matched up.  These are new grains.
   */
  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
    for (auto feature_num = decltype(_feature_sets[map_num].size())(0);
         feature_num < _feature_sets[map_num].size(); ++feature_num)
      // If it's not in the index list, it hasn't been transferred
      if (new_grain_idx_to_existing_grain_idx.find(std::make_pair(map_num, feature_num)) == new_grain_idx_to_existing_grain_idx.end())
      {
        mooseAssert(_feature_sets[map_num][i]._status == NOT_MARKED, "Feature in wrong state, logic error");

        auto new_idx = _unique_grains.size();

        _feature_sets[map_num][feature_num]._status = MARKED;                       // Mark it
        _unique_grains[new_idx] = std::move(_feature_sets[map_num][feature_num]);   // transfer ownership

        // Trigger the callback
        newGrainCreated(new_idx);
      }

  /**
   * Finally we need to mark any grains in the unique list that aren't marked as inactive.  These are the unique grains
   * that didn't match up to any new feature. This should only happen if it's the last active grain for
   * this particular variable.
   */
  for (auto & grain_pair : _unique_grains)
    if (grain_pair.second._status == NOT_MARKED)
    {
      _console << "Marking Grain " << grain_pair.first << " as INACTIVE (variable index: "
               << grain_pair.second._var_idx <<  ")\n"
               << grain_pair.second;
      grain_pair.second._status = INACTIVE;
    }
}

void
GrainTracker::newGrainCreated(unsigned int new_grain_idx)
{
  _console << COLOR_YELLOW
           << "*****************************************************************************\n"
           << "Couldn't find a matching grain while working on variable index: " << _unique_grains[new_grain_idx]._var_idx
           << "\nCreating new unique grain: " << new_grain_idx << '\n' << _unique_grains[new_grain_idx]
           << "\n*****************************************************************************\n" << COLOR_DEFAULT;
}

void
GrainTracker::remapGrains()
{
  // Don't remap grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  _console << "Running remap Grains" << std::endl;

  /**
   * The remapping algorithm is recursive. We will reuse the "merged" flag in the FeatureData
   * to track which grains are currently being remapped so we don't have runaway recursion.
   * TODO: Possibly rename this variable?
   */
  for (auto & grain_pair : _unique_grains)
    grain_pair.second._merged = false;

  /**
   * Loop over each grain and see if any grains represented by the same variable are "touching"
   */
  bool grains_remapped;
  do
  {
    grains_remapped = false;
    for (auto grain_it1 = _unique_grains.begin(); grain_it1 != _unique_grains.end(); ++grain_it1)
    {
      if (grain_it1->second._status == INACTIVE)
        continue;

      for (auto grain_it2 = _unique_grains.begin(); grain_it2 != _unique_grains.end(); ++grain_it2)
      {
        // Don't compare a grain with itself and don't try to remap inactive grains
        if (grain_it1 == grain_it2 || grain_it2->second._status == INACTIVE)
          continue;

        if (grain_it1->second._var_idx == grain_it2->second._var_idx &&   // Are the grains represented by the same variable?
            grain_it1->second.isStichable(grain_it2->second) &&           // If so, do their bboxes intersect (coarse level check)?
            setsIntersect(grain_it1->second._halo_ids.begin(),            // If so, do they actually overlap (tight "hull" check)?
                          grain_it1->second._halo_ids.end(),
                          grain_it2->second._halo_ids.begin(),
                          grain_it2->second._halo_ids.end()))
        {
          Moose::out
            << COLOR_YELLOW
            << "Grain #" << grain_it1->first << " intersects Grain #" << grain_it2->first
            << " (variable index: " << grain_it1->second._var_idx << ")\n"
            << COLOR_DEFAULT;

          for (unsigned int max = 0; max <= _max_renumbering_recursion; ++max)
            if (max < _max_renumbering_recursion)
            {
              if (attemptGrainRenumber(grain_it1->second, grain_it1->first, 0, max) || attemptGrainRenumber(grain_it2->second, grain_it2->first, 0, max))
                break;
            }
            else if (!attemptGrainRenumber(grain_it1->second, grain_it1->first, 0, max) && !attemptGrainRenumber(grain_it2->second, grain_it2->first, 0, max))
              mooseError(COLOR_RED << "Unable to find any suitable grains for remapping. Perhaps you need more op variables?\n\n" << COLOR_DEFAULT);

          grains_remapped = true;
        }
      }
    }
  }
  while (grains_remapped);
}

void
GrainTracker::computeMinDistancesFromGrain(FeatureData & grain,
                                           std::vector<std::list<GrainDistance> > & min_distances)
{
  /**
   * TODO: I should be able to sort these even better. Negative distances could represent the number
   * of grains overlapping the current grain with the same index.
   *
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
    if (grain_pair.second._status == INACTIVE || grain_pair.second._var_idx == grain._var_idx)
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

  std::map<Node *, CacheValues> cache;

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

#ifndef NDEBUG
  _console << "\n********************************************\nDistances list for grain " << grain_id << '\n';
  for (unsigned int i = 0; i < min_distances.size(); ++i)
  {
    for (const auto & grain_distance : min_distances[i])
      _console << grain_distance._distance << ": " << grain_distance._grain_id << ": " <<  grain_distance._var_index << '\n';
    _console << '\n';
  }
#endif

  for (unsigned int i = 0; i < min_distances.size(); ++i)
  {
    const auto target_it = min_distances[i].begin();
    if (target_it == min_distances[i].end())
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

      swapSolutionValues(grain, target_it->_var_index, cache, BYPASS, depth);
      return true;
    }

    // If the distance isn't positive we just need to make sure that none of the grains represented by the
    // target variable index would intersect this one if we were to remap
    auto next_target_it = target_it;
    bool intersection_hit = false;
    std::ostringstream oss;
    while (!intersection_hit && next_target_it != min_distances[i].end())
    {
      if (next_target_it->_distance > 0)
        break;

      mooseAssert(_unique_grains.find(next_target_it->_grain_id) != _unique_grains.end(), "Error in indexing target grain in attemptGrainRenumber");
      FeatureData & next_target_grain = _unique_grains[next_target_it->_grain_id];

      // If any grains touch we're done here
      if (setsIntersect(grain._halo_ids.begin(), grain._halo_ids.end(),
                        next_target_grain._halo_ids.begin(), next_target_grain._halo_ids.end()))
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

      swapSolutionValues(grain, target_it->_var_index, cache, BYPASS, depth);
      return true;
    }

    // If we reach this part of the loop, there is no simple renumbering that can be done.
    mooseAssert(_unique_grains.find(target_it->_grain_id) != _unique_grains.end(), "Error in indexing target grain in attemptGrainRenumber");
    FeatureData & target_grain = _unique_grains[target_it->_grain_id];

    // Make sure this grain isn't marked. If it is, we can't recurse here
    if (target_grain._merged)
      return false;

    // Save the solution values in case we overright them during recursion
    swapSolutionValues(grain, target_it->_var_index, cache, FILL, depth);

    // TODO: Make sure this distance is -1 or higher or fine intersections only exist for a single variable
    // Propose a new variable index for the current grain and recurse
    grain._var_idx = target_it->_var_index;
    grain._merged = true;
    if (attemptGrainRenumber(target_grain, target_it->_grain_id, depth+1, max))
    {
      // SUCCESS!
      Moose::out
        << COLOR_GREEN
        << "- Depth " << depth << ": Remapping grain #" << grain_id << " from variable index " << curr_var_idx
        << " to " << target_it->_var_index << '\n'
        << COLOR_DEFAULT;

      // NOTE: swapSolutionValues currently reads the current variable index off the grain. We need to set
      //       back here before calling this method.
      grain._var_idx = curr_var_idx;
      swapSolutionValues(grain, target_it->_var_index, cache, USE, depth);

      return true;
    }
    else
      // Need to set our var index back after failed recursive step
      grain._var_idx = curr_var_idx;

    // Always "unmark" the grain after the recursion so it can be used by other remap operations
    grain._merged = false;
  }

  return false;
}

void
GrainTracker::swapSolutionValues(FeatureData & grain, unsigned int var_idx, std::map<Node *, CacheValues> & cache,
                                 REMAP_CACHE_MODE cache_mode, unsigned int depth)
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
          updated_nodes_tmp.insert(curr_node);         // cache this node so we don't attempt to remap it again within this loop
          swapSolutionValuesHelper(curr_node, grain._var_idx, var_idx, cache, cache_mode);
        }
      }
    }
    else
      swapSolutionValuesHelper(mesh.query_node_ptr(entity), grain._var_idx, var_idx, cache, cache_mode);
  }

  // Update the variable index in the unique grain datastructure
  grain._var_idx = var_idx;

  // Close all of the solution vectors (we only need to do this once after all swaps are complete)
  if (depth == 0)
  {
    _nl.solution().close();
    _nl.solutionOld().close();
    _nl.solutionOlder().close();

    _fe_problem.getNonlinearSystem().sys().update();
  }
}

void
GrainTracker::swapSolutionValuesHelper(Node * curr_node, unsigned int curr_var_idx, unsigned int new_var_idx, std::map<Node *, CacheValues> & cache,
                                       REMAP_CACHE_MODE cache_mode)
{
  if (curr_node && curr_node->processor_id() == processor_id())
  {
    // Reinit the node so we can get and set values of the solution here
    _subproblem.reinitNode(curr_node, 0);

    // Get the value out of the solution vector or the cache
    Real current, old, older;

    if (cache_mode == FILL || cache_mode == BYPASS)
    {
      current = _vars[curr_var_idx]->nodalSln()[0];
      old = _vars[curr_var_idx]->nodalSlnOld()[0];
      older = _vars[curr_var_idx]->nodalSlnOlder()[0];
    }
    else // USE
    {
      const auto cache_it = cache.find(curr_node);
      mooseAssert(cache_it != cache.end(), "Error in cache");
      current = cache_it->second.current;
      old = cache_it->second.old;
      older = cache_it->second.older;
    }

    // Cache the value or use it!
    if (cache_mode == FILL)
    {
      cache[curr_node].current = current;
      cache[curr_node].old = old;
      cache[curr_node].older = older;
    }
    else // USE or BYPASS
    {
      {
        const auto & dof_index = _vars[new_var_idx]->nodalDofIndex();

        // Transfer this solution from the current to the new
        _nl.solution().set(dof_index, current);
        _nl.solutionOld().set(dof_index, old);
        _nl.solutionOlder().set(dof_index, older);
      }
      {
        const auto & dof_index = _vars[curr_var_idx]->nodalDofIndex();

        // Set the DOF for the current variable to zero
        _nl.solution().set(dof_index, 0.0);
        _nl.solutionOld().set(dof_index, 0.0);
        _nl.solutionOlder().set(dof_index, 0.0);
      }
    }
  }
}

void
GrainTracker::updateFieldInfo()
{
  for (auto map_num = decltype(_maps_size)(0); map_num < _maps_size; ++map_num)
    _feature_maps[map_num].clear();

  _halo_ids.clear();

  std::map<unsigned int, Real> tmp_map;
  MeshBase & mesh = _mesh.getMesh();

  for (const auto & grain_pair : _unique_grains)
  {
    unsigned int curr_var = grain_pair.second._var_idx;
    unsigned int map_idx = (_single_map_mode || _condense_map_info) ? 0 : curr_var;

    if (grain_pair.second._status == INACTIVE)
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
      _halo_ids[entity] = grain_pair.second._var_idx;

    for (auto entity : grain_pair.second._ghosted_ids)
      _ghosted_entity_ids[entity] = grain_pair.first;
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

void
GrainTracker::calculateBubbleVolumes()
{
  Moose::perf_log.push("calculateBubbleVolumes()", "GrainTracker");

  // The size of the bubble array will be sized to the max index of the unique grains map
  unsigned int max_id = _unique_grains.size() ? _unique_grains.rbegin()->first + 1: 0;
  _all_feature_volumes.resize(max_id, 0);

  const MeshBase::const_element_iterator el_end = _mesh.getMesh().active_local_elements_end();
  for (MeshBase::const_element_iterator el = _mesh.getMesh().active_local_elements_begin(); el != el_end; ++el)
  {
    Elem * elem = *el;
    auto elem_n_nodes = elem->n_nodes();
    auto curr_volume = elem->volume();

    for (auto & grain_pair : _unique_grains)
    {
      if (grain_pair.second._status == INACTIVE)
        continue;

      if (_is_elemental)
      {
        auto elem_id = elem->id();
        if (grain_pair.second._local_ids.find(elem_id) != grain_pair.second._local_ids.end())
        {
          mooseAssert(grain_pair.first < _all_feature_volumes.size(), "_all_feature_volumes access out of bounds");
          _all_feature_volumes[grain_pair.first] += curr_volume;
          break;
        }
      }
      else
      {
        // Count the number of nodes on this element which are flooded.
        unsigned int flooded_nodes = 0;
        for (unsigned int node = 0; node < elem_n_nodes; ++node)
        {
          auto node_id = elem->node(node);
          if (grain_pair.second._local_ids.find(node_id) != grain_pair.second._local_ids.end())
            ++flooded_nodes;
        }

        // If a majority of the nodes for this element are flooded,
        // assign its volume to the current bubble_counter entry.
        if (flooded_nodes >= elem_n_nodes / 2)
          _all_feature_volumes[grain_pair.first] += curr_volume;
      }
    }
  }

  // do all the sums!
  _communicator.sum(_all_feature_volumes);

  Moose::perf_log.pop("calculateBubbleVolumes()", "GrainTracker");
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
