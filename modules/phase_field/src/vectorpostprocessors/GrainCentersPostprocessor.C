/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GrainCentersPostprocessor.h"
#include "GrainTrackerInterface.h"

template<>
InputParameters validParams<GrainCentersPostprocessor>()
{
  InputParameters params = validParams<VectorPostprocessor>();
  params.addClassDescription("Outputs the values from GrainCentersPostprocessor");
  params.addParam<UserObjectName>("grain_data","Specify user object that gives center of mass and volume of grains");
  return params;
}

GrainCentersPostprocessor::GrainCentersPostprocessor(const InputParameters & parameters) :
    GeneralVectorPostprocessor(parameters),
    _grain_volume_center_vector(declareVector("grain_volume_center_vector")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_data"))
{
}

void
GrainCentersPostprocessor::execute()
{
  auto num_grains = _grain_tracker.getNumberGrains();
  _grain_volume_center_vector.resize(4 * num_grains);

  for (auto i = decltype(num_grains)(0); i < num_grains; ++i)
  {
    auto volume = _grain_tracker.getGrainVolume(i);
    auto centroid = _grain_tracker.getGrainCentroid(i);

    _grain_volume_center_vector[4 * i + 0] = volume;
    _grain_volume_center_vector[4 * i + 1] = centroid(0);
    _grain_volume_center_vector[4 * i + 2] = centroid(1);
    _grain_volume_center_vector[4 * i + 3] = centroid(2);
  }
}
