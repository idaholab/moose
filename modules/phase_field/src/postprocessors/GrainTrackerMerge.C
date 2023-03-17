#include "GrainTrackerMerge.h"

registerMooseObject("PhaseFieldApp", GrainTrackerMerge);

InputParameters
GrainTrackerMerge::validParams()
{
  InputParameters params = GrainTrackerMerge::validParams();
  params.addClassDescription("Grain Tracker derived object for merging of grains based on misorientation angle.");
  params.addRequiredParam<UserObjectName>("euler_angle_provider",
                                          "Name of Euler angle provider user object");

  return params;
}

GrainTrackerMerge::GrainTrackerMerge(const InputParameters & parameters)
  : GrainTracker(parameters),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider"))
{
  std::cout << "the done of GrainTrackerMerge" << std::endl;
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
      auto & grain_j = _feature_sets[grain_i._adjacent_id[grain_num_j]];

      if (grain_j._status == Status::INACTIVE || grain_i._id >= grain_j._id)
        continue;

      EulerAngles angles_j = _euler.getEulerAngles(grain_j._id);

      misor_angle = MisorientationAngleCalculator::calculateMisorientaion(angles_i, angles_j, _s_misorientation_angle)._misor;

      if (misor_angle < 0.8)
      {
        _console << COLOR_YELLOW << "Grain #" << grain_i._id << " and Grain #" << grain_j._id
                 << " was merged (misor: " << misor_angle << ").\n"
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

    if (grain_i._status == Status::INACTIVE)
      continue;

    for (const auto grain_num_j : index_range(_feature_sets))
    {
      auto & grain_j = _feature_sets[grain_num_j];

      if (grain_i._id < grain_j._id && grain_j._status != Status::INACTIVE 
          && grain_i.boundingBoxesIntersect(grain_j) && grain_i.halosIntersect(grain_j))
      {
        grain_i._adjacent_id.push_back(grain_j._id); // It must be noted that the number stored in _adjacent_id is _feature_sets[i],
        grain_j._adjacent_id.push_back(grain_i._id); // and the ID of the adjacent grain is _feature_sets[i]._id
      }
    }

    std::sort(grain_i._adjacent_id.begin(), grain_i._adjacent_id.end());
  }
}