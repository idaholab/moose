#include "GrainTrackerMerge.h"

registerMooseObject("PhaseFieldApp", GrainTrackerMerge);

InputParameters
GrainTrackerMerge::validParams()
{
  InputParameters params = GrainTracker::validParams();
  params.addClassDescription("Grain Tracker derived object for merging of grains based on misorientation angle.");
  params.addRequiredParam<UserObjectName>("euler_angle_provider",
                                          "Name of Euler angle provider user object");

  return params;
}

GrainTrackerMerge::GrainTrackerMerge(const InputParameters & parameters)
  : GrainTracker(parameters),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider"))
{
}

void
GrainTrackerMerge::mergeGrainsBasedMisorientation()
{
  createAdjacentIDVector();

  Real misor_angle = 0;

  for (const auto grain_num_i : index_range(_feature_sets))
  {
    if (_feature_sets[grain_num_i]._status == Status::INACTIVE)
      continue;

    auto & grain_i = _feature_sets[grain_num_i];
    EulerAngles angles_i = _euler.getEulerAngles(grain_i._id);
    for (const auto grain_num_j : index_range(grain_i._adjacent_id))
    {

      // Obtain the feature set index corresponding to the grain ID 
      auto & grain_j_index  = _feature_id_to_index_maps[grain_i._adjacent_id[grain_num_j]];

      auto & grain_j = _feature_sets[grain_j_index];

      if (grain_j._status == Status::INACTIVE || grain_i._id >= grain_j._id)
        continue;

      EulerAngles angles_j = _euler.getEulerAngles(grain_j._id);

      misor_angle = MisorientationAngleCalculator::calculateMisorientaion(angles_i, angles_j, _s_misorientation_angle)._misor;

      if (misor_angle < 0.8)
      {
        _console << COLOR_YELLOW << "Grain #" << grain_i._id << " and Grain #" 
                 << grain_j._id  << " was merged (misor: " << misor_angle << ").\n"
                 << COLOR_DEFAULT;

        grain_j._id = grain_i._id;
      }
    }
  }
}

void 
GrainTrackerMerge::createAdjacentIDVector()
{

  for (const auto grain_num_i : index_range(_feature_sets))
  {
    auto & grain_i = _feature_sets[grain_num_i];

    _feature_id_to_index_maps[grain_i._id] = grain_num_i;

    if (grain_i._status == Status::INACTIVE)
      continue;

    for (const auto grain_num_j : index_range(_feature_sets))
    {
      auto & grain_j = _feature_sets[grain_num_j];

      if (grain_i._id < grain_j._id && grain_j._status != Status::INACTIVE 
          && grain_i.boundingBoxesIntersect(grain_j) && grain_i.halosIntersect(grain_j))
      {
        grain_i._adjacent_id.push_back(grain_j._id); // It should be noted here that feature ID is stored,
        grain_j._adjacent_id.push_back(grain_i._id); // not _feature_sets index
      }
    }

    std::sort(grain_i._adjacent_id.begin(), grain_i._adjacent_id.end());
  }
}

void 
GrainTrackerMerge::remapMisorientedGrains()
{
  // This data structure is used to store the mapping from Grain ID to new variable index
  std::map<unsigned int, std::size_t> grain_id_to_new_var_merge_grain;

  if (_is_primary)
  {
    for (const auto i : index_range(_feature_sets))
    {
      auto & grain1 = _feature_sets[i];

      for (const auto j : index_range(_feature_sets))
      {
        auto & grain2 = _feature_sets[j];
        if (i == j)
          continue;

        if (i < j && grain1._id == grain2._id)
        {
          if (grain1._var_index != grain2._var_index)
          {
            if (_verbosity_level > 0)
              _console << COLOR_YELLOW << "Re-merge Grain (#" << grain1._id << ") detected on unmatched OPs (" << grain1._var_index << ", "
                       << grain2._var_index << ") attempting to remap to " << grain1._var_index
                       << ".\n"
                       << COLOR_DEFAULT;

            grain_id_to_new_var_merge_grain.emplace_hint(
                grain_id_to_new_var_merge_grain.end(),
                std::pair<unsigned int, std::size_t>(grain2._id, grain1._var_index));
          }
        }
      }
    }
  } // root processor

  // Communicate the std::map to all ranks
  _communicator.broadcast(grain_id_to_new_var_merge_grain);

  // Perform swaps if any occurred
  if (!grain_id_to_new_var_merge_grain.empty())
  {
    // Cache for holding values during swaps
    std::vector<std::map<Node *, CacheValues>> cache(_n_vars);

    // Perform the actual swaps on all processors
    for (auto & grain : _feature_sets)
    {
      // See if this grain was remapped
      auto new_var_it = grain_id_to_new_var_merge_grain.find(grain._id);
      if (new_var_it != grain_id_to_new_var_merge_grain.end())
        swapSolutionValues(grain, new_var_it->second, cache, RemapCacheMode::FILL);
    }

    for (auto & grain : _feature_sets)
    {
      // See if this grain was remapped
      auto new_var_it = grain_id_to_new_var_merge_grain.find(grain._id);
      if (new_var_it != grain_id_to_new_var_merge_grain.end())
        swapSolutionValues(grain, new_var_it->second, cache, RemapCacheMode::USE);
    }

    _console << COLOR_GREEN << "Swaps complete in remapMisorientedGrains" << std::endl;
    _console << std::endl;
  }  
}