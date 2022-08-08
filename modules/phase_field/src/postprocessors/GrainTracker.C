//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainTracker.h"

// MOOSE includes
#include "PolycrystalUserObjectBase.h"
#include "GeneratedMesh.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "NonlinearSystem.h"

#include "libmesh/periodic_boundary_base.h"

// C++ includes
#include <algorithm>
#include <limits>
#include <numeric>

template <>
void
dataStore(std::ostream & stream, GrainTracker::PartialFeatureData & feature, void * context)
{
  storeHelper(stream, feature.boundary_intersection, context);
  storeHelper(stream, feature.id, context);
  storeHelper(stream, feature.centroid, context);
  storeHelper(stream, feature.status, context);
}

template <>
void
dataLoad(std::istream & stream, GrainTracker::PartialFeatureData & feature, void * context)
{
  loadHelper(stream, feature.boundary_intersection, context);
  loadHelper(stream, feature.id, context);
  loadHelper(stream, feature.centroid, context);
  loadHelper(stream, feature.status, context);
}

registerMooseObject("PhaseFieldApp", GrainTracker);

InputParameters
GrainTracker::validParams()
{
  InputParameters params = FeatureFloodCount::validParams();
  params += GrainTrackerInterface::validParams();

  // FeatureFloodCount adds a relationship manager, but we need to extend that for GrainTracker
  params.clearRelationshipManagers();

  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC,

      [](const InputParameters & obj_params, InputParameters & rm_params)
      { rm_params.set<unsigned short>("layers") = obj_params.get<unsigned short>("halo_level"); }

  );

  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::ALGEBRAIC);

  // The GrainTracker requires non-volatile storage for tracking grains across invocations.
  params.set<bool>("restartable_required") = true;

  params.addClassDescription("Grain Tracker object for running reduced order parameter simulations "
                             "without grain coalescence.");

  return params;
}

GrainTracker::GrainTracker(const InputParameters & parameters)
  : FeatureFloodCount(parameters),
    GrainTrackerInterface(),
    _tracking_step(getParam<int>("tracking_step")),
    _halo_level(getParam<unsigned short>("halo_level")),
    _max_remap_recursion_depth(getParam<unsigned short>("max_remap_recursion_depth")),
    _n_reserve_ops(getParam<unsigned short>("reserve_op")),
    _reserve_op_index(_n_reserve_ops <= _n_vars ? _n_vars - _n_reserve_ops : 0),
    _reserve_op_threshold(getParam<Real>("reserve_op_threshold")),
    _remap(getParam<bool>("remap_grains")),
    _tolerate_failure(getParam<bool>("tolerate_failure")),
    _nl(_fe_problem.getNonlinearSystemBase()),
    _poly_ic_uo(parameters.isParamValid("polycrystal_ic_uo")
                    ? &getUserObject<PolycrystalUserObjectBase>("polycrystal_ic_uo")
                    : nullptr),
    _verbosity_level(getParam<short>("verbosity_level")),
    _first_time(declareRestartableData<bool>("first_time", true)),
    _error_on_grain_creation(getParam<bool>("error_on_grain_creation")),
    _reserve_grain_first_index(0),
    _old_max_grain_id(0),
    _max_curr_grain_id(declareRestartableData<unsigned int>("max_curr_grain_id", invalid_id)),
    _is_transient(_subproblem.isTransient())
{
  if (_tolerate_failure)
    paramInfo("tolerate_failure",
              "Tolerate failure has been set to true. Non-physical simulation results "
              "are possible, you will be notified in the event of a failed remapping operation.");

  if (_tracking_step > 0 && _poly_ic_uo)
    mooseError("Can't start tracking after the initial condition when using a polycrystal_ic_uo");
}

GrainTracker::~GrainTracker() {}

Real
GrainTracker::getEntityValue(dof_id_type entity_id,
                             FieldType field_type,
                             std::size_t var_index) const
{
  if (_t_step < _tracking_step)
    return 0;

  return FeatureFloodCount::getEntityValue(entity_id, field_type, var_index);
}

const std::vector<unsigned int> &
GrainTracker::getVarToFeatureVector(dof_id_type elem_id) const
{
  return FeatureFloodCount::getVarToFeatureVector(elem_id);
}

unsigned int
GrainTracker::getFeatureVar(unsigned int feature_id) const
{
  return FeatureFloodCount::getFeatureVar(feature_id);
}

std::size_t
GrainTracker::getNumberActiveGrains() const
{
  // Note: This value is parallel consistent, see FeatureFloodCount::communicateAndMerge()
  return _feature_count;
}

std::size_t
GrainTracker::getTotalFeatureCount() const
{
  // Note: This value is parallel consistent, see assignGrains()/trackGrains()
  return _max_curr_grain_id == invalid_id ? 0 : _max_curr_grain_id + 1;
}

Point
GrainTracker::getGrainCentroid(unsigned int grain_id) const
{
  mooseAssert(grain_id < _feature_id_to_local_index.size(), "Grain ID out of bounds");
  auto grain_index = _feature_id_to_local_index[grain_id];

  if (grain_index != invalid_size_t)
  {
    mooseAssert(_feature_id_to_local_index[grain_id] < _feature_sets.size(),
                "Grain index out of bounds");
    // Note: This value is parallel consistent, see GrainTracker::broadcastAndUpdateGrainData()
    return _feature_sets[_feature_id_to_local_index[grain_id]]._centroid;
  }

  // Inactive grain
  return Point();
}

bool
GrainTracker::doesFeatureIntersectBoundary(unsigned int feature_id) const
{
  // TODO: This data structure may need to be turned into a Multimap
  mooseAssert(feature_id < _feature_id_to_local_index.size(), "Grain ID out of bounds");

  auto feature_index = _feature_id_to_local_index[feature_id];
  if (feature_index != invalid_size_t)
  {
    mooseAssert(feature_index < _feature_sets.size(), "Grain index out of bounds");
    return _feature_sets[feature_index]._boundary_intersection != BoundaryIntersection::NONE;
  }

  return false;
}

bool
GrainTracker::doesFeatureIntersectSpecifiedBoundary(unsigned int feature_id) const
{
  // TODO: This data structure may need to be turned into a Multimap
  mooseAssert(feature_id < _feature_id_to_local_index.size(), "Grain ID out of bounds");

  auto feature_index = _feature_id_to_local_index[feature_id];
  if (feature_index != invalid_size_t)
  {
    mooseAssert(feature_index < _feature_sets.size(), "Grain index out of bounds");
    return ((_feature_sets[feature_index]._boundary_intersection &
             BoundaryIntersection::SPECIFIED_BOUNDARY) == BoundaryIntersection::SPECIFIED_BOUNDARY);
  }

  return false;
}

bool
GrainTracker::isFeaturePercolated(unsigned int feature_id) const
{
  // TODO: This data structure may need to be turned into a Multimap
  mooseAssert(feature_id < _feature_id_to_local_index.size(), "Grain ID out of bounds");

  auto feature_index = _feature_id_to_local_index[feature_id];
  if (feature_index != invalid_size_t)
  {
    mooseAssert(feature_index < _feature_sets.size(), "Grain index out of bounds");
    bool primary = ((_feature_sets[feature_index]._boundary_intersection &
                     BoundaryIntersection::PRIMARY_PERCOLATION_BOUNDARY) ==
                    BoundaryIntersection::PRIMARY_PERCOLATION_BOUNDARY);
    bool secondary = ((_feature_sets[feature_index]._boundary_intersection &
                       BoundaryIntersection::SECONDARY_PERCOLATION_BOUNDARY) ==
                      BoundaryIntersection::SECONDARY_PERCOLATION_BOUNDARY);
    return (primary && secondary);
  }

  return false;
}

void
GrainTracker::initialize()
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  /**
   * If we are passed the first time, we need to save the existing grains before beginning the
   * tracking on the current step. We'll do that with a swap since the _feature_sets contents will
   * be cleared anyway.
   */
  if (!_first_time)
    _feature_sets_old.swap(_feature_sets);

  FeatureFloodCount::initialize();
}

void
GrainTracker::meshChanged()
{
  // Update the element ID ranges for use when computing halo maps
  if (_compute_halo_maps && _mesh.isDistributedMesh())
  {
    _all_ranges.clear();

    auto range = std::make_pair(std::numeric_limits<dof_id_type>::max(),
                                std::numeric_limits<dof_id_type>::min());
    for (const auto & current_elem : _mesh.getMesh().active_local_element_ptr_range())
    {
      auto id = current_elem->id();
      if (id < range.first)
        range.first = id;
      else if (id > range.second)
        range.second = id;
    }

    _communicator.gather(0, range, _all_ranges);
  }

  FeatureFloodCount::meshChanged();
}

void
GrainTracker::execute()
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  if (_poly_ic_uo && _first_time)
    return;

  FeatureFloodCount::execute();
}

Real
GrainTracker::getThreshold(std::size_t var_index) const
{
  // If we are inspecting a reserve op parameter, we need to make sure
  // that there is an entity above the reserve_op threshold before
  // starting the flood of the feature.
  if (var_index >= _reserve_op_index)
    return _reserve_op_threshold;
  else
    return _step_threshold;
}

void
GrainTracker::prepopulateState(const FeatureFloodCount & ffc_object)
{
  mooseAssert(_first_time, "This method should only be called on the first invocation");

  _feature_sets.clear();

  /**
   * The minimum information needed to bootstrap the GrainTracker is as follows:
   * _feature_sets
   * _feature_count
   */
  if (_is_primary)
  {
    const auto & features = ffc_object.getFeatures();
    for (auto & feature : features)
      _feature_sets.emplace_back(feature.duplicate());

    _feature_count = _feature_sets.size();
  }
  else
  {
    const auto & features = ffc_object.getFeatures();
    _partial_feature_sets[0].clear();
    for (auto & feature : features)
      _partial_feature_sets[0].emplace_back(feature.duplicate());
  }

  // Make sure that feature count is communicated to all ranks
  _communicator.broadcast(_feature_count);
}

void
GrainTracker::finalize()
{
  // Don't track grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  TIME_SECTION("finalize", 3, "Finalizing GrainTracker");

  // Expand the depth of the halos around all grains
  auto num_halo_layers = _halo_level >= 1
                             ? _halo_level - 1
                             : 0; // The first level of halos already exists so subtract one

  if (_poly_ic_uo && _first_time)
    prepopulateState(*_poly_ic_uo);
  else
  {
    expandEdgeHalos(num_halo_layers);

    // Build up the grain map on the root processor
    communicateAndMerge();
  }

  /**
   * Assign or Track Grains
   */
  if (_first_time)
    assignGrains();
  else
    trackGrains();

  if (_verbosity_level > 1)
    _console << "Finished inside of trackGrains" << std::endl;

  /**
   * Broadcast essential data
   */
  broadcastAndUpdateGrainData();

  /**
   * Remap Grains
   */
  if (_remap)
    remapGrains();

  updateFieldInfo();
  if (_verbosity_level > 1)
    _console << "Finished inside of updateFieldInfo" << std::endl;

  // Set the first time flag false here (after all methods of finalize() have completed)
  _first_time = false;

  // TODO: Release non essential memory
  if (_verbosity_level > 0)
    _console << "Finished inside of GrainTracker\n" << std::endl;
}

void
GrainTracker::broadcastAndUpdateGrainData()
{
  TIME_SECTION("broadcastAndUpdateGrainData", 3, "Broadcasting and Updating Grain Data");

  std::vector<PartialFeatureData> root_feature_data;
  std::vector<std::string> send_buffer(1), recv_buffer;

  if (_is_primary)
  {
    root_feature_data.reserve(_feature_sets.size());

    // Populate a subset of the information in a small data structure
    std::transform(_feature_sets.begin(),
                   _feature_sets.end(),
                   std::back_inserter(root_feature_data),
                   [](FeatureData & feature)
                   {
                     PartialFeatureData partial_feature;
                     partial_feature.boundary_intersection = feature._boundary_intersection;
                     partial_feature.id = feature._id;
                     partial_feature.centroid = feature._centroid;
                     partial_feature.status = feature._status;
                     return partial_feature;
                   });

    std::ostringstream oss;
    dataStore(oss, root_feature_data, this);
    send_buffer[0].assign(oss.str());
  }

  // Broadcast the data to all ranks
  _communicator.broadcast_packed_range((void *)(nullptr),
                                       send_buffer.begin(),
                                       send_buffer.end(),
                                       (void *)(nullptr),
                                       std::back_inserter(recv_buffer));

  // Unpack and update
  if (!_is_primary)
  {
    std::istringstream iss;
    iss.str(recv_buffer[0]);
    iss.clear();

    dataLoad(iss, root_feature_data, this);

    for (const auto & partial_data : root_feature_data)
    {
      // See if this processor has a record of this grain
      if (partial_data.id < _feature_id_to_local_index.size() &&
          _feature_id_to_local_index[partial_data.id] != invalid_size_t)
      {
        auto & grain = _feature_sets[_feature_id_to_local_index[partial_data.id]];
        grain._boundary_intersection = partial_data.boundary_intersection;
        grain._centroid = partial_data.centroid;
        if (partial_data.status == Status::INACTIVE)
          grain._status = Status::INACTIVE;
      }
    }
  }
}

void
GrainTracker::assignGrains()
{
  mooseAssert(_first_time, "assignGrains may only be called on the first tracking step");

  /**
   * We need to assign grainIDs to get the simulation going. We'll use the default sorting that
   * doesn't require valid grainIDs (relies on _min_entity_id and _var_index). These will be the
   * unique grain numbers that we must track for remainder of the simulation.
   */
  if (_is_primary)
  {
    // Find the largest grain ID, this requires sorting if the ID is not already set
    sortAndLabel();

    if (_feature_sets.empty())
    {
      _max_curr_grain_id = invalid_id;
      _reserve_grain_first_index = 0;
    }
    else
    {
      _max_curr_grain_id = _feature_sets.back()._id;
      _reserve_grain_first_index = _max_curr_grain_id + 1;
    }

    for (auto & grain : _feature_sets)
      grain._status = Status::MARKED; // Mark the grain

  } // is_primary

  /*************************************************************
   ****************** COLLECTIVE WORK SECTION ******************
   *************************************************************/

  // Make IDs on all non-primary ranks consistent
  scatterAndUpdateRanks();

  // Build up an id to index map
  _communicator.broadcast(_max_curr_grain_id);
  buildFeatureIdToLocalIndices(_max_curr_grain_id);

  // Now trigger the newGrainCreated() callback on all ranks
  if (_max_curr_grain_id != invalid_id)
    for (unsigned int new_id = 0; new_id <= _max_curr_grain_id; ++new_id)
      newGrainCreated(new_id);
}

void
GrainTracker::trackGrains()
{
  TIME_SECTION("trackGrains", 3, "Tracking Grains");

  mooseAssert(!_first_time, "Track grains may only be called when _tracking_step > _t_step");

  // Used to track indices for which to trigger the new grain callback on (used on all ranks)
  auto _old_max_grain_id = _max_curr_grain_id;

  /**
   * Only the primary rank does tracking, the remaining ranks
   * wait to receive local to global indices from the primary.
   */
  if (_is_primary)
  {
    // Reset Status on active unique grains
    std::vector<unsigned int> map_sizes(_maps_size);
    for (auto & grain : _feature_sets_old)
    {
      if (grain._status != Status::INACTIVE)
      {
        grain._status = Status::CLEAR;
        map_sizes[grain._var_index]++;
      }
    }

    // Print out stats on overall tracking changes per var_index
    if (_verbosity_level > 0)
    {
      _console << "\nGrain Tracker Status:";
      for (const auto map_num : make_range(_maps_size))
      {
        _console << "\nGrains active index " << map_num << ": " << map_sizes[map_num] << " -> "
                 << _feature_counts_per_map[map_num];
        if (map_sizes[map_num] > _feature_counts_per_map[map_num])
          _console << "--";
        else if (map_sizes[map_num] < _feature_counts_per_map[map_num])
          _console << "++";
      }
      _console << '\n' << std::endl;
    }

    // Before we track grains, lets sort them so that we get parallel consistent answers
    std::sort(_feature_sets.begin(), _feature_sets.end());

    /**
     * To track grains across time steps, we will loop over our unique grains and link each one up
     * with one of our new unique grains. The criteria for doing this will be to find the unique
     * grain in the new list with a matching variable index whose centroid is closest to this
     * unique grain.
     */
    std::vector<std::size_t> new_grain_index_to_existing_grain_index(_feature_sets.size(),
                                                                     invalid_size_t);

    for (const auto old_grain_index : index_range(_feature_sets_old))
    {
      auto & old_grain = _feature_sets_old[old_grain_index];

      if (old_grain._status == Status::INACTIVE) // Don't try to find matches for inactive grains
        continue;

      std::size_t closest_match_index = invalid_size_t;
      Real min_centroid_diff = std::numeric_limits<Real>::max();

      /**
       * The _feature_sets vector is constructed by _var_index so we can avoid looping over all
       * indices. We can quickly jump to the first matching index to reduce the number of
       * comparisons and terminate our loop when our variable index stops matching.
       */
      // clang-format off
      auto start_it =
          std::lower_bound(_feature_sets.begin(), _feature_sets.end(), old_grain._var_index,
                           [](const FeatureData & item, std::size_t var_index)
                           {
                             return item._var_index < var_index;
                           });
      // clang-format on

      // We only need to examine grains that have matching variable indices
      bool any_boxes_intersect = false;
      for (MooseIndex(_feature_sets)
               new_grain_index = std::distance(_feature_sets.begin(), start_it);
           new_grain_index < _feature_sets.size() &&
           _feature_sets[new_grain_index]._var_index == old_grain._var_index;
           ++new_grain_index)
      {
        auto & new_grain = _feature_sets[new_grain_index];

        /**
         * Don't try to do any matching unless the bounding boxes at least overlap. This is to avoid
         * the corner case of having a grain split and a grain disappear during the same time step!
         */
        if (new_grain.boundingBoxesIntersect(old_grain))
        {
          any_boxes_intersect = true;
          Real curr_centroid_diff = centroidRegionDistance(old_grain._bboxes, new_grain._bboxes);
          if (curr_centroid_diff <= min_centroid_diff)
          {
            closest_match_index = new_grain_index;
            min_centroid_diff = curr_centroid_diff;
          }
        }
      }

      if (_verbosity_level > 2 && !any_boxes_intersect)
        _console << "\nNo intersecting bounding boxes found while trying to match grain "
                 << old_grain;

      // found a match
      if (closest_match_index != invalid_size_t)
      {
        /**
         * It's possible that multiple existing grains will map to a single new grain (indicated by
         * finding multiple matches when we are building this map). This will happen any time a
         * grain disappears during this time step. We need to figure out the rightful owner in this
         * case and inactivate the old grain.
         */
        auto curr_index = new_grain_index_to_existing_grain_index[closest_match_index];
        if (curr_index != invalid_size_t)
        {
          // The new feature being competed for
          auto & new_grain = _feature_sets[closest_match_index];

          // The other old grain competing to match up to the same new grain
          auto & other_old_grain = _feature_sets_old[curr_index];

          auto centroid_diff1 = centroidRegionDistance(new_grain._bboxes, old_grain._bboxes);
          auto centroid_diff2 = centroidRegionDistance(new_grain._bboxes, other_old_grain._bboxes);

          auto & inactive_grain = (centroid_diff1 < centroid_diff2) ? other_old_grain : old_grain;

          inactive_grain._status = Status::INACTIVE;
          if (_verbosity_level > 0)
          {
            _console << COLOR_GREEN << "Marking Grain " << inactive_grain._id
                     << " as INACTIVE (variable index: " << inactive_grain._var_index << ")\n"
                     << COLOR_DEFAULT;
            if (_verbosity_level > 1)
              _console << inactive_grain;
          }

          /**
           * If the grain we just marked inactive was the one whose index was in the new grain
           * to existing grain map (other_old_grain). Then we need to update the map to point
           * to the new match winner.
           */
          if (&inactive_grain == &other_old_grain)
            new_grain_index_to_existing_grain_index[closest_match_index] = old_grain_index;
        }
        else
          new_grain_index_to_existing_grain_index[closest_match_index] = old_grain_index;
      }
    }

    // Mark all resolved grain matches
    for (const auto new_index : index_range(new_grain_index_to_existing_grain_index))
    {
      auto curr_index = new_grain_index_to_existing_grain_index[new_index];

      // This may be a new grain, we'll handle that case below
      if (curr_index == invalid_size_t)
        continue;

      mooseAssert(_feature_sets_old[curr_index]._id != invalid_id,
                  "Invalid ID in old grain structure");

      _feature_sets[new_index]._id = _feature_sets_old[curr_index]._id; // Transfer ID
      _feature_sets[new_index]._status = Status::MARKED;      // Mark the status in the new set
      _feature_sets_old[curr_index]._status = Status::MARKED; // Mark the status in the old set
    }

    /**
     * At this point we have should have only two cases left to handle:
     * Case 1: A grain in the new set who has an unset status (These are new grains, previously
     *         untracked) This case is easy to understand. Since we are matching up grains by
     *         looking at the old set and finding closest matches in the new set, any grain in
     *         the new set that isn't matched up is simply new since some other grain satisfied
     *         each and every request from the old set.
     *
     * Case 2: A grain in the old set who has an unset status (These are inactive grains that
     *         haven't been marked) We can only fall into this case when the very last grain on
     *         a given variable disappears during the current time step. In that case we never have
     *         a matching _var_index in the comparison loop above so that old grain never competes
     *         for any new grain which means it can't be marked inactive in the loop above.
     */
    // Case 1 (new grains in _feature_sets):
    for (const auto grain_num : index_range(_feature_sets))
    {
      auto & grain = _feature_sets[grain_num];

      // New Grain
      if (grain._status == Status::CLEAR)
      {
        /**
         * Now we need to figure out what kind of "new" grain this is. Is it a nucleating grain that
         * we're just barely seeing for the first time or is it a "splitting" grain. A grain that
         * gets pinched into two or more pieces usually as it is being absorbed by other grains or
         * possibly due to external forces. We have to handle splitting grains this way so as to
         * no confuse them with regular grains that just happen to be in contact in this step.
         *
         * Splitting Grain: An grain that is unmatched by any old grain
         *                  on the same order parameter with touching halos.
         *
         * Nucleating Grain: A completely new grain appearing somewhere in the domain
         *                   not overlapping any other grain's halo.
         *
         * To figure out which case we are dealing with, we have to make another pass over all of
         * the existing grains with matching variable indices to see if any of them have overlapping
         * halos.
         */

        // clang-format off
        auto start_it =
            std::lower_bound(_feature_sets.begin(), _feature_sets.end(), grain._var_index,
                             [](const FeatureData & item, std::size_t var_index)
                             {
                               return item._var_index < var_index;
                             });
        // clang-format on

        // Loop over matching variable indices
        for (MooseIndex(_feature_sets)
                 new_grain_index = std::distance(_feature_sets.begin(), start_it);
             new_grain_index < _feature_sets.size() &&
             _feature_sets[new_grain_index]._var_index == grain._var_index;
             ++new_grain_index)
        {
          auto & other_grain = _feature_sets[new_grain_index];

          // Splitting grain?
          if (grain_num != new_grain_index && // Make sure indices aren't pointing at the same grain
              other_grain._status == Status::MARKED && // and that the other grain is indeed marked
              other_grain.boundingBoxesIntersect(grain) && // and the bboxes intersect
              other_grain.halosIntersect(grain))           // and the halos also intersect
          // TODO: Inspect combined volume and see if it's "close" to the expected value
          {
            grain._id = other_grain._id;    // Set the duplicate ID
            grain._status = Status::MARKED; // Mark it

            if (_verbosity_level > 0)
              _console << COLOR_YELLOW << "Split Grain Detected #" << grain._id
                       << " (variable index: " << grain._var_index << ")\n"
                       << COLOR_DEFAULT;
            if (_verbosity_level > 1)
              _console << grain << other_grain;
          }
        }

        if (grain._var_index < _reserve_op_index)
        {
          /**
           * The "try-harder loop":
           * OK so we still have an extra grain in the new set that isn't matched up against the
           * old set and since the order parameter isn't reserved. We aren't really expecting a new
           * grain. Let's try to make a few more attempts to see if this is a split grain even
           * though it failed to match the criteria above. This might happen if the halo front is
           * advancing too fast!
           *
           * In this loop we'll make an attempt to match up this new grain to the old halos. If
           * adaptivity is happening this could fail as elements in the new set may be at a
           * different level than in the old set. If we get multiple matches, we'll compare the
           * grain volumes (based on elements, not integrated to choose the closest).
           *
           * Future ideas:
           * Look at the volume fraction of the new grain and overlay it over the volume fraction
           * of the old grain (would require more saved information, or an aux field hanging around
           * (subject to projection problems).
           */
          if (_verbosity_level > 1)
            _console << COLOR_YELLOW
                     << "Trying harder to detect a split grain while examining grain on variable "
                        "index "
                     << grain._var_index << '\n'
                     << COLOR_DEFAULT;

          std::vector<std::size_t> old_grain_indices;
          for (const auto old_grain_index : index_range(_feature_sets_old))
          {
            auto & old_grain = _feature_sets_old[old_grain_index];

            if (old_grain._status == Status::INACTIVE)
              continue;

            /**
             * Note that the old grains we are looking at will already be marked from the earlier
             * tracking phase. We are trying to see if this unmatched grain is part of a larger
             * whole. To do that we'll look at the halos across the time step.
             */
            if (grain._var_index == old_grain._var_index &&
                grain.boundingBoxesIntersect(old_grain) && grain.halosIntersect(old_grain))
              old_grain_indices.push_back(old_grain_index);
          }

          if (old_grain_indices.size() == 1)
          {
            grain._id = _feature_sets_old[old_grain_indices[0]]._id;
            grain._status = Status::MARKED;

            if (_verbosity_level > 0)
              _console << COLOR_YELLOW << "Split Grain Detected #" << grain._id
                       << " (variable index: " << grain._var_index << ")\n"
                       << COLOR_DEFAULT;
          }
          else if (old_grain_indices.size() > 1)
            _console
                << COLOR_RED << "Split Grain Likely Detected #" << grain._id
                << " Need more information to find correct candidate - contact a developer!\n\n"
                << COLOR_DEFAULT;
        }

        // Must be a nucleating grain (status is still not set)
        if (grain._status == Status::CLEAR)
        {
          auto new_index = getNextUniqueID();
          grain._id = new_index;          // Set the ID
          grain._status = Status::MARKED; // Mark it

          if (_verbosity_level > 0)
            _console << COLOR_YELLOW << "Nucleating Grain Detected "
                     << " (variable index: " << grain._var_index << ")\n"
                     << COLOR_DEFAULT;
          if (_verbosity_level > 1)
            _console << grain;
        }
      }
    }

    // Case 2 (inactive grains in _feature_sets_old)
    for (auto & grain : _feature_sets_old)
    {
      if (grain._status == Status::CLEAR)
      {
        grain._status = Status::INACTIVE;
        if (_verbosity_level > 0)
        {
          _console << COLOR_GREEN << "Marking Grain " << grain._id
                   << " as INACTIVE (variable index: " << grain._var_index << ")\n"
                   << COLOR_DEFAULT;
          if (_verbosity_level > 1)
            _console << grain;
        }
      }
    }
  } // is_primary

  /*************************************************************
   ****************** COLLECTIVE WORK SECTION ******************
   *************************************************************/

  // Make IDs on all non-primary ranks consistent
  scatterAndUpdateRanks();

  // Build up an id to index map
  _communicator.broadcast(_max_curr_grain_id);
  buildFeatureIdToLocalIndices(_max_curr_grain_id);

  /**
   * Trigger callback for new grains
   */
  if (_old_max_grain_id < _max_curr_grain_id)
  {
    for (auto new_id = _old_max_grain_id + 1; new_id <= _max_curr_grain_id; ++new_id)
    {
      // Don't trigger the callback on the reserve IDs
      if (new_id >= _reserve_grain_first_index + _n_reserve_ops)
      {
        // See if we've been instructed to terminate with an error
        if (!_first_time && _error_on_grain_creation)
          mooseError(
              "Error: New grain detected and \"error_on_new_grain_creation\" is set to true");
        else
          newGrainCreated(new_id);
      }
    }
  }
}

void
GrainTracker::newGrainCreated(unsigned int new_grain_id)
{
  if (!_first_time && _is_primary)
  {
    mooseAssert(new_grain_id < _feature_id_to_local_index.size(), "new_grain_id is out of bounds");
    auto grain_index = _feature_id_to_local_index[new_grain_id];
    mooseAssert(grain_index != invalid_size_t && grain_index < _feature_sets.size(),
                "new_grain_id appears to be invalid");

    const auto & grain = _feature_sets[grain_index];
    _console << COLOR_YELLOW
             << "\n*****************************************************************************"
             << "\nCouldn't find a matching grain while working on variable index: "
             << grain._var_index << "\nCreating new unique grain: " << new_grain_id << '\n'
             << grain
             << "\n*****************************************************************************\n"
             << COLOR_DEFAULT;
  }
}

std::vector<unsigned int>
GrainTracker::getNewGrainIDs() const
{
  std::vector<unsigned int> new_ids(_max_curr_grain_id - _old_max_grain_id);
  auto new_id = _old_max_grain_id + 1;

  // Generate the new ids
  std::iota(new_ids.begin(), new_ids.end(), new_id);

  return new_ids;
}

void
GrainTracker::remapGrains()
{
  // Don't remap grains if the current simulation step is before the specified tracking step
  if (_t_step < _tracking_step)
    return;

  TIME_SECTION("remapGrains", 3, "Remapping Grains");

  if (_verbosity_level > 1)
    _console << "Running remap Grains\n" << std::endl;

  /**
   * Map used for communicating remap indices to all ranks
   * This map isn't populated until after the remap loop.
   * It's declared here before we enter the root scope
   * since it's needed by all ranks during the broadcast.
   */
  std::map<unsigned int, std::size_t> grain_id_to_new_var;

  // Items are added to this list when split grains are found
  std::list<std::pair<std::size_t, std::size_t>> split_pairs;

  /**
   * The remapping algorithm is recursive. We will use the status variable in each FeatureData
   * to track which grains are currently being remapped so we don't have runaway recursion.
   * To begin we need to clear all of the active (MARKED) flags (CLEAR).
   *
   * Additionally we need to record each grain's variable index so that we can communicate
   * changes to the non-root ranks later in a single batch.
   */
  if (_is_primary)
  {
    // Build the map to detect difference in _var_index mappings after the remap operation
    std::map<unsigned int, std::size_t> grain_id_to_existing_var_index;
    for (auto & grain : _feature_sets)
    {
      // Unmark the grain so it can be used in the remap loop
      grain._status = Status::CLEAR;

      grain_id_to_existing_var_index[grain._id] = grain._var_index;
    }

    // Make sure that all split pieces of any grain are on the same OP
    for (const auto i : index_range(_feature_sets))
    {
      auto & grain1 = _feature_sets[i];

      for (const auto j : index_range(_feature_sets))
      {
        auto & grain2 = _feature_sets[j];
        if (i == j)
          continue;

        // The first condition below is there to prevent symmetric checks (duplicate values)
        if (i < j && grain1._id == grain2._id)
        {
          split_pairs.push_front(std::make_pair(i, j));
          if (grain1._var_index != grain2._var_index)
          {
            if (_verbosity_level > 0)
              _console << COLOR_YELLOW << "Split Grain (#" << grain1._id
                       << ") detected on unmatched OPs (" << grain1._var_index << ", "
                       << grain2._var_index << ") attempting to remap to " << grain1._var_index
                       << ".\n"
                       << COLOR_DEFAULT;

            /**
             * We're not going to try very hard to look for a suitable remapping. Just set it to
             * what we want and hope it all works out. Make the GrainTracker great again!
             */
            grain1._var_index = grain2._var_index;
            grain1._status |= Status::DIRTY;
          }
        }
      }
    }

    /**
     * Loop over each grain and see if any grains represented by the same variable are "touching"
     */
    bool any_grains_remapped = false;
    bool grains_remapped;

    std::set<unsigned int> notify_ids;
    do
    {
      grains_remapped = false;
      notify_ids.clear();

      for (auto & grain1 : _feature_sets)
      {
        // We need to remap any grains represented on any variable index above the cuttoff
        if (grain1._var_index >= _reserve_op_index)
        {
          if (_verbosity_level > 0)
            _console << COLOR_YELLOW << "\nGrain #" << grain1._id
                     << " detected on a reserved order parameter #" << grain1._var_index
                     << ", remapping to another variable\n"
                     << COLOR_DEFAULT;

          for (const auto max : make_range(0, _max_remap_recursion_depth + 1))
            if (max < _max_remap_recursion_depth)
            {
              if (attemptGrainRenumber(grain1, 0, max))
                break;
            }
            else if (!attemptGrainRenumber(grain1, 0, max))
            {
              _console << std::flush;
              std::stringstream oss;
              oss << "Unable to find any suitable order parameters for remapping while working "
                  << "with Grain #" << grain1._id << ", which is on a reserve order parameter.\n"
                  << "\n\nPossible Resolutions:\n"
                  << "\t- Add more order parameters to your simulation (8 for 2D, 28 for 3D)\n"
                  << "\t- Increase adaptivity or reduce your grain boundary widths\n"
                  << "\t- Make sure you are not starting with too many grains for the mesh size\n";
              mooseError(oss.str());
            }

          grains_remapped = true;
        }

        for (auto & grain2 : _feature_sets)
        {
          // Don't compare a grain with itself and don't try to remap inactive grains
          if (&grain1 == &grain2)
            continue;

          if (grain1._var_index == grain2._var_index && // grains represented by same variable?
              grain1._id != grain2._id &&               // are they part of different grains?
              grain1.boundingBoxesIntersect(grain2) &&  // do bboxes intersect (coarse level)?
              grain1.halosIntersect(grain2))            // do they actually overlap (fine level)?
          {
            if (_verbosity_level > 0)
              _console << COLOR_YELLOW << "Grain #" << grain1._id << " intersects Grain #"
                       << grain2._id << " (variable index: " << grain1._var_index << ")\n"
                       << COLOR_DEFAULT;

            for (const auto max : make_range(0, _max_remap_recursion_depth + 1))
            {
              if (max < _max_remap_recursion_depth)
              {
                if (attemptGrainRenumber(grain1, 0, max))
                {
                  grains_remapped = true;
                  break;
                }
              }
              else if (!attemptGrainRenumber(grain1, 0, max) &&
                       !attemptGrainRenumber(grain2, 0, max))
              {
                notify_ids.insert(grain1._id);
                notify_ids.insert(grain2._id);
              }
            }
          }
        }
      }
      any_grains_remapped |= grains_remapped;
    } while (grains_remapped);

    if (!notify_ids.empty())
    {
      _console << std::flush;
      std::stringstream oss;
      oss << "Unable to find any suitable order parameters for remapping while working "
          << "with the following grain IDs:\n"
          << Moose::stringify(notify_ids, ", ", "", true) << "\n\nPossible Resolutions:\n"
          << "\t- Add more order parameters to your simulation (8 for 2D, 28 for 3D)\n"
          << "\t- Increase adaptivity or reduce your grain boundary widths\n"
          << "\t- Make sure you are not starting with too many grains for the mesh size\n";

      if (_tolerate_failure)
        mooseWarning(oss.str());
      else
        mooseError(oss.str());
    }

    // Verify that split grains are still intact
    for (auto & split_pair : split_pairs)
      if (_feature_sets[split_pair.first]._var_index != _feature_sets[split_pair.first]._var_index)
        mooseError("Split grain remapped - This case is currently not handled");

    /**
     * The remapping loop is complete but only on the primary process.
     * Now we need to build the remap map and communicate it to the
     * remaining processors.
     */
    for (auto & grain : _feature_sets)
    {
      mooseAssert(grain_id_to_existing_var_index.find(grain._id) !=
                      grain_id_to_existing_var_index.end(),
                  "Missing unique ID");

      auto old_var_index = grain_id_to_existing_var_index[grain._id];

      if (old_var_index != grain._var_index)
      {
        mooseAssert(static_cast<bool>(grain._status & Status::DIRTY), "grain status is incorrect");

        grain_id_to_new_var.emplace_hint(
            grain_id_to_new_var.end(),
            std::pair<unsigned int, std::size_t>(grain._id, grain._var_index));

        /**
         * Since the remapping algorithm only runs on the root process,
         * the variable index on the primary's grains is inconsistent from
         * the rest of the ranks. These are the grains with a status of
         * DIRTY. As we build this map we will temporarily switch these
         * variable indices back to the correct value so that all
         * processors use the same algorithm to remap.
         */
        grain._var_index = old_var_index;
        // Clear the DIRTY status as well for consistency
        grain._status &= ~Status::DIRTY;
      }
    }

    if (!grain_id_to_new_var.empty())
    {
      if (_verbosity_level > 1)
      {
        _console << "Final remapping tally:\n";
        for (const auto & remap_pair : grain_id_to_new_var)
          _console << "Grain #" << remap_pair.first << " var_index "
                   << grain_id_to_existing_var_index[remap_pair.first] << " -> "
                   << remap_pair.second << '\n';
        _console << "Communicating swaps with remaining processors..." << std::endl;
      }
    }
  } // root processor

  // Communicate the std::map to all ranks
  _communicator.broadcast(grain_id_to_new_var);

  // Perform swaps if any occurred
  if (!grain_id_to_new_var.empty())
  {
    // Cache for holding values during swaps
    std::vector<std::map<Node *, CacheValues>> cache(_n_vars);

    // Perform the actual swaps on all processors
    for (auto & grain : _feature_sets)
    {
      // See if this grain was remapped
      auto new_var_it = grain_id_to_new_var.find(grain._id);
      if (new_var_it != grain_id_to_new_var.end())
        swapSolutionValues(grain, new_var_it->second, cache, RemapCacheMode::FILL);
    }

    for (auto & grain : _feature_sets)
    {
      // See if this grain was remapped
      auto new_var_it = grain_id_to_new_var.find(grain._id);
      if (new_var_it != grain_id_to_new_var.end())
        swapSolutionValues(grain, new_var_it->second, cache, RemapCacheMode::USE);
    }

    _nl.solution().close();
    _nl.solutionOld().close();
    _nl.solutionOlder().close();

    _fe_problem.getNonlinearSystemBase().system().update();

    if (_verbosity_level > 1)
      _console << "Swaps complete" << std::endl;
  }
}

void
GrainTracker::computeMinDistancesFromGrain(FeatureData & grain,
                                           std::vector<std::list<GrainDistance>> & min_distances)
{
  /**
   * In the diagram below assume we have 4 order parameters. The grain with the asterisk needs to
   * be remapped. All order parameters are used in neighboring grains. For all "touching" grains,
   * the value of the corresponding entry in min_distances will be a negative integer representing
   * the number of immediate neighbors with that order parameter.
   *
   *  Note: Only the first member of the pair (the distance) is shown in the array below.
   *        e.g. [-2.0, -max, -1.0, -2.0]
   *
   * After sorting, variable index 2 (value: -1.0) be at the end of the array and will be the first
   * variable we attempt to renumber the current grain to.
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
  for (const auto i : index_range(_feature_sets))
  {
    auto & other_grain = _feature_sets[i];

    if (other_grain._var_index == grain._var_index || other_grain._var_index >= _reserve_op_index)
      continue;

    auto target_var_index = other_grain._var_index;
    auto target_grain_index = i;
    auto target_grain_id = other_grain._id;

    Real curr_bbox_diff = boundingRegionDistance(grain._bboxes, other_grain._bboxes);

    GrainDistance grain_distance_obj(
        curr_bbox_diff, target_var_index, target_grain_index, target_grain_id);

    // To handle touching halos we penalize the top pick each time we see another
    if (curr_bbox_diff == -1.0 && !min_distances[target_var_index].empty())
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

  /**
   * See if we have any completely open OPs (excluding reserve order parameters) or the order
   * parameter corresponding to this grain, we need to put them in the list or the grain  tracker
   * won't realize that those vars are available for remapping.
   */
  for (const auto var_index : make_range(_reserve_op_index))
  {
    // Don't put an entry in for matching variable indices (i.e. we can't remap to ourselves)
    if (grain._var_index == var_index)
      continue;

    if (min_distances[var_index].empty())
      min_distances[var_index].emplace_front(std::numeric_limits<Real>::max(), var_index);
  }
}

bool
GrainTracker::attemptGrainRenumber(FeatureData & grain, unsigned int depth, unsigned int max_depth)
{
  // End the recursion of our breadth first search
  if (depth > max_depth)
    return false;

  std::size_t curr_var_index = grain._var_index;

  std::vector<std::map<Node *, CacheValues>> cache;

  std::vector<std::list<GrainDistance>> min_distances(_vars.size());

  /**
   * We have two grains that are getting close represented by the same order parameter.
   * We need to map to the variable whose closest grain to this one is furthest away by bounding
   * region to bounding region distance.
   */
  computeMinDistancesFromGrain(grain, min_distances);

  /**
   * We have a vector of the distances to the closest grains represented by each of our variables.
   * We just need to pick a suitable grain to replace with. We will start with the maximum of this
   * this list: (max of the mins), but will settle for next to largest and so forth as we make more
   * attempts at remapping grains. This is a graph coloring problem so more work will be required
   * to optimize this process.
   *
   * Note: We don't have an explicit check here to avoid remapping a variable to itself. This is
   * unnecessary since the min_distance of a variable is explicitly set up above.
   */
  // clang-format off
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
  // clang-format on

  for (auto & list_ref : min_distances)
  {
    const auto target_it = list_ref.begin();
    if (target_it == list_ref.end())
      continue;

    // If the distance is positive we can just remap and be done
    if (target_it->_distance > 0)
    {
      if (_verbosity_level > 0)
      {
        _console << COLOR_GREEN << "- Depth " << depth << ": Remapping grain #" << grain._id
                 << " from variable index " << curr_var_index << " to " << target_it->_var_index;
        if (target_it->_distance == std::numeric_limits<Real>::max())
          _console << " which currently contains zero grains.\n\n" << COLOR_DEFAULT;
        else
          _console << " whose closest grain (#" << target_it->_grain_id << ") is at a distance of "
                   << std::sqrt(target_it->_distance) << "\n\n"
                   << COLOR_DEFAULT;
      }

      grain._status |= Status::DIRTY;
      grain._var_index = target_it->_var_index;
      return true;
    }

    // If the distance isn't positive we just need to make sure that none of the grains represented
    // by the target variable index would intersect this one if we were to remap
    {
      auto next_target_it = target_it;
      bool intersection_hit = false;
      unsigned short num_close_targets = 0;
      std::ostringstream oss;
      while (!intersection_hit && next_target_it != list_ref.end())
      {
        if (next_target_it->_distance > 0)
          break;

        mooseAssert(next_target_it->_grain_index < _feature_sets.size(),
                    "Error in indexing target grain in attemptGrainRenumber");
        FeatureData & next_target_grain = _feature_sets[next_target_it->_grain_index];

        // If any grains touch we're done here
        if (grain.halosIntersect(next_target_grain))
          intersection_hit = true;
        else
        {
          if (num_close_targets > 0)
            oss << ", "; // delimiter
          oss << "#" << next_target_it->_grain_id;
        }

        ++next_target_it;
        ++num_close_targets;
      }

      if (!intersection_hit)
      {
        if (_verbosity_level > 0)
        {
          _console << COLOR_GREEN << "- Depth " << depth << ": Remapping grain #" << grain._id
                   << " from variable index " << curr_var_index << " to " << target_it->_var_index;

          if (num_close_targets == 1)
            _console << " whose closest grain (" << oss.str()
                     << ") is inside our bounding box but whose halo is not touching.\n\n"
                     << COLOR_DEFAULT;
          else
            _console << " whose closest grains (" << oss.str()
                     << ") are inside our bounding box but whose halos are not touching.\n\n"
                     << COLOR_DEFAULT;
        }

        grain._status |= Status::DIRTY;
        grain._var_index = target_it->_var_index;
        return true;
      }
    }

    // If we reach this part of the loop, there is no simple renumbering that can be done.
    mooseAssert(target_it->_grain_index < _feature_sets.size(),
                "Error in indexing target grain in attemptGrainRenumber");
    FeatureData & target_grain = _feature_sets[target_it->_grain_index];

    /**
     * If we get to this case and the best distance is less than -1, we are in big trouble.
     * This means that grains represented by all of the remaining order parameters are
     * overlapping this one in at least two places. We'd have to maintain multiple recursive
     * chains, or just start over from scratch...
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
    grain._var_index = target_it->_var_index;
    grain._status |= Status::MARKED;
    if (attemptGrainRenumber(target_grain, depth + 1, max_depth))
    {
      // SUCCESS!
      if (_verbosity_level > 0)
        _console << COLOR_GREEN << "- Depth " << depth << ": Remapping grain #" << grain._id
                 << " from variable index " << curr_var_index << " to " << target_it->_var_index
                 << "\n\n"
                 << COLOR_DEFAULT;

      // Now we need to mark the grain as DIRTY since the recursion succeeded.
      grain._status |= Status::DIRTY;
      return true;
    }
    else
      // FAILURE, We need to set our var index back after failed recursive step
      grain._var_index = curr_var_index;

    // ALWAYS "unmark" (or clear the MARKED status) after recursion so it can be used by other remap
    // operations
    grain._status &= ~Status::MARKED;
  }

  return false;
}

void
GrainTracker::swapSolutionValues(FeatureData & grain,
                                 std::size_t new_var_index,
                                 std::vector<std::map<Node *, CacheValues>> & cache,
                                 RemapCacheMode cache_mode)
{
  MeshBase & mesh = _mesh.getMesh();

  // Remap the grain
  std::set<Node *> updated_nodes_tmp; // Used only in the elemental case
  for (auto entity : grain._local_ids)
  {
    if (_is_elemental)
    {
      Elem * elem = mesh.query_elem_ptr(entity);
      if (!elem)
        continue;

      for (unsigned int i = 0; i < elem->n_nodes(); ++i)
      {
        Node * curr_node = elem->node_ptr(i);
        if (updated_nodes_tmp.find(curr_node) == updated_nodes_tmp.end())
        {
          // cache this node so we don't attempt to remap it again within this loop
          updated_nodes_tmp.insert(curr_node);
          swapSolutionValuesHelper(curr_node, grain._var_index, new_var_index, cache, cache_mode);
        }
      }
    }
    else
      swapSolutionValuesHelper(
          mesh.query_node_ptr(entity), grain._var_index, new_var_index, cache, cache_mode);
  }

  // Update the variable index in the unique grain datastructure after swaps are complete
  if (cache_mode == RemapCacheMode::USE || cache_mode == RemapCacheMode::BYPASS)
    grain._var_index = new_var_index;
}

void
GrainTracker::swapSolutionValuesHelper(Node * curr_node,
                                       std::size_t curr_var_index,
                                       std::size_t new_var_index,
                                       std::vector<std::map<Node *, CacheValues>> & cache,
                                       RemapCacheMode cache_mode)
{
  if (curr_node && curr_node->processor_id() == processor_id())
  {
    // Reinit the node so we can get and set values of the solution here
    _subproblem.reinitNode(curr_node, 0);

    // Local variables to hold values being transferred
    Real current, old = 0, older = 0;
    // Retrieve the value either from the old variable or cache
    if (cache_mode == RemapCacheMode::FILL || cache_mode == RemapCacheMode::BYPASS)
    {
      current = _vars[curr_var_index]->dofValues()[0];
      if (_is_transient)
      {
        old = _vars[curr_var_index]->dofValuesOld()[0];
        older = _vars[curr_var_index]->dofValuesOlder()[0];
      }
    }
    else // USE
    {
      const auto cache_it = cache[curr_var_index].find(curr_node);
      mooseAssert(cache_it != cache[curr_var_index].end(), "Error in cache");
      current = cache_it->second.current;
      old = cache_it->second.old;
      older = cache_it->second.older;
    }

    // Cache the value or use it!
    if (cache_mode == RemapCacheMode::FILL)
    {
      cache[curr_var_index][curr_node].current = current;
      cache[curr_var_index][curr_node].old = old;
      cache[curr_var_index][curr_node].older = older;
    }
    else // USE or BYPASS
    {
      const auto & dof_index = _vars[new_var_index]->nodalDofIndex();

      // Transfer this solution from the old to the current
      _nl.solution().set(dof_index, current);
      if (_is_transient)
      {
        _nl.solutionOld().set(dof_index, old);
        _nl.solutionOlder().set(dof_index, older);
      }
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
      const auto & dof_index = _vars[curr_var_index]->nodalDofIndex();

      // Set the DOF for the current variable to zero
      _nl.solution().set(dof_index, 0.0);
      if (_is_transient)
      {
        _nl.solutionOld().set(dof_index, 0.0);
        _nl.solutionOlder().set(dof_index, 0.0);
      }
    }
  }
}

void
GrainTracker::updateFieldInfo()
{
  TIME_SECTION("updateFieldInfo", 3, "Updating Field Info");

  for (const auto map_num : make_range(_maps_size))
    _feature_maps[map_num].clear();

  std::map<dof_id_type, Real> tmp_map;

  for (const auto & grain : _feature_sets)
  {
    std::size_t curr_var = grain._var_index;
    std::size_t map_index = (_single_map_mode || _condense_map_info) ? 0 : curr_var;

    for (auto entity : grain._local_ids)
    {
      // Highest variable value at this entity wins
      Real entity_value = std::numeric_limits<Real>::lowest();
      if (_is_elemental)
      {
        const Elem * elem = _mesh.elemPtr(entity);
        std::vector<Point> centroid(1, elem->vertex_average());
        if (_poly_ic_uo && _first_time)
        {
          entity_value = _poly_ic_uo->getVariableValue(grain._var_index, centroid[0]);
        }
        else
        {
          _fe_problem.reinitElemPhys(elem, centroid, 0);
          entity_value = _vars[curr_var]->sln()[0];
        }
      }
      else
      {
        auto node_ptr = _mesh.nodePtr(entity);
        entity_value = _vars[curr_var]->getNodalValue(*node_ptr);
      }

      if (entity_value != std::numeric_limits<Real>::lowest() &&
          (tmp_map.find(entity) == tmp_map.end() || entity_value > tmp_map[entity]))
      {
        mooseAssert(grain._id != invalid_id, "Missing Grain ID");
        _feature_maps[map_index][entity] = grain._id;

        if (_var_index_mode)
          _var_index_maps[map_index][entity] = grain._var_index;

        tmp_map[entity] = entity_value;
      }

      if (_compute_var_to_feature_map)
      {
        auto insert_pair = moose_try_emplace(
            _entity_var_to_features, entity, std::vector<unsigned int>(_n_vars, invalid_id));
        auto & vec_ref = insert_pair.first->second;

        if (insert_pair.second)
        {
          // insert the reserve op numbers (if appropriate)
          for (const auto reserve_index : make_range(_n_reserve_ops))
            vec_ref[reserve_index] = _reserve_grain_first_index + reserve_index;
        }
        vec_ref[grain._var_index] = grain._id;
      }
    }

    if (_compute_halo_maps)
      for (auto entity : grain._halo_ids)
        _halo_ids[grain._var_index][entity] = grain._var_index;

    for (auto entity : grain._ghosted_ids)
      _ghosted_entity_ids[entity] = 1;
  }

  communicateHaloMap();
}

void
GrainTracker::communicateHaloMap()
{
  if (_compute_halo_maps)
  {
    // rank               var_index    entity_id
    std::vector<std::pair<std::size_t, dof_id_type>> halo_ids_all;

    std::vector<int> counts;
    std::vector<std::pair<std::size_t, dof_id_type>> local_halo_ids;
    std::size_t counter = 0;

    const bool isDistributedMesh = _mesh.isDistributedMesh();

    if (_is_primary)
    {
      std::vector<std::vector<std::pair<std::size_t, dof_id_type>>> root_halo_ids(_n_procs);
      counts.resize(_n_procs);

      // Loop over the _halo_ids "field" and build minimal lists for all of the other ranks
      for (const auto var_index : index_range(_halo_ids))
      {
        for (const auto & entity_pair : _halo_ids[var_index])
        {
          auto entity_id = entity_pair.first;
          if (isDistributedMesh)
          {
            // Check to see which contiguous range this entity ID falls into
            auto range_it =
                std::lower_bound(_all_ranges.begin(),
                                 _all_ranges.end(),
                                 entity_id,
                                 [](const std::pair<dof_id_type, dof_id_type> range,
                                    dof_id_type entity_id) { return range.second < entity_id; });

            mooseAssert(range_it != _all_ranges.end(), "No range round?");

            // Recover the index from the iterator
            auto proc_id = std::distance(_all_ranges.begin(), range_it);

            // Now add this halo entity to the map for the corresponding proc to scatter latter
            root_halo_ids[proc_id].push_back(std::make_pair(var_index, entity_id));
          }
          else
          {
            DofObject * halo_entity;
            if (_is_elemental)
              halo_entity = _mesh.queryElemPtr(entity_id);
            else
              halo_entity = _mesh.queryNodePtr(entity_id);

            if (halo_entity)
              root_halo_ids[halo_entity->processor_id()].push_back(
                  std::make_pair(var_index, entity_id));
          }
        }
      }

      // Build up the counts vector for MPI scatter
      std::size_t global_count = 0;
      for (const auto & vector_ref : root_halo_ids)
      {
        std::copy(vector_ref.begin(), vector_ref.end(), std::back_inserter(halo_ids_all));
        counts[counter] = vector_ref.size();
        global_count += counts[counter++];
      }
    }

    _communicator.scatter(halo_ids_all, counts, local_halo_ids);

    // Now add the contributions from the root process to the processor local maps
    for (const auto & halo_pair : local_halo_ids)
      _halo_ids[halo_pair.first].emplace(std::make_pair(halo_pair.second, halo_pair.first));

    /**
     * Finally remove halo markings from interior regions. This step is necessary because we expand
     * halos _before_ we do communication but that expansion can and will likely go into the
     * interior of the grain (from a single processor's perspective). We could expand halos after
     * merging, but that would likely be less scalable.
     */
    for (const auto & grain : _feature_sets)
      for (auto local_id : grain._local_ids)
        _halo_ids[grain._var_index].erase(local_id);
  }
}

Real
GrainTracker::centroidRegionDistance(std::vector<BoundingBox> & bboxes1,
                                     std::vector<BoundingBox> & bboxes2) const
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
GrainTracker::boundingRegionDistance(std::vector<BoundingBox> & bboxes1,
                                     std::vector<BoundingBox> & bboxes2) const
{
  /**
   * The region that each grain covers is represented by a bounding box large enough to encompassing
   * all the points within that grain. When using periodic boundaries, we may have several discrete
   * "pieces" of a grain each represented by a bounding box. The distance between any two grains
   * is defined as the minimum distance between any pair of boxes, one selected from each grain.
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

unsigned int
GrainTracker::getNextUniqueID()
{
  /**
   * Get the next unique grain ID but make sure to respect
   * reserve ids. Note, that the first valid ID for a new
   * grain is _reserve_grain_first_index + _n_reserve_ops because
   * _reserve_grain_first_index IS a valid index. It does not
   * point to the last valid index of the non-reserved grains.
   */
  _max_curr_grain_id = std::max(_max_curr_grain_id == invalid_id ? 0 : _max_curr_grain_id + 1,
                                _reserve_grain_first_index + _n_reserve_ops /* no +1 here!*/);

  return _max_curr_grain_id;
}

/*************************************************
 ************** Helper Structures ****************
 ************************************************/
GrainDistance::GrainDistance(Real distance, std::size_t var_index)
  : GrainDistance(distance,
                  var_index,
                  std::numeric_limits<std::size_t>::max(),
                  std::numeric_limits<unsigned int>::max())
{
}

GrainDistance::GrainDistance(Real distance,
                             std::size_t var_index,
                             std::size_t grain_index,
                             unsigned int grain_id)
  : _distance(distance), _var_index(var_index), _grain_index(grain_index), _grain_id(grain_id)
{
}

bool
GrainDistance::operator<(const GrainDistance & rhs) const
{
  return _distance < rhs._distance;
}
