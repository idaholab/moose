/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GrainCentersPostprocessor.h"
#include "ComputeGrainCenterUserObject.h"

template <>
InputParameters
validParams<GrainCentersPostprocessor>()
{
  InputParameters params = validParams<VectorPostprocessor>();
  params.addClassDescription("Outputs the values from GrainCentersPostprocessor");
  params.addParam<UserObjectName>(
      "grain_data", "Specify user object that gives center of mass and volume of grains");
  return params;
}

GrainCentersPostprocessor::GrainCentersPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _grain_volume_center_vector(declareVector("grain_volume_center_vector")),
    _grain_data(getUserObject<ComputeGrainCenterUserObject>("grain_data")),
    _grain_volumes(_grain_data.getGrainVolumes()),
    _grain_centers(_grain_data.getGrainCenters()),
    _total_grains(_grain_volumes.size())
{
  _grain_volume_center_vector.resize(_total_grains * 4);
}

void
GrainCentersPostprocessor::execute()
{
  for (unsigned int i = 0; i < _total_grains; ++i)
  {
    _grain_volume_center_vector[4 * i + 0] = _grain_volumes[i];
    _grain_volume_center_vector[4 * i + 1] = _grain_centers[i](0);
    _grain_volume_center_vector[4 * i + 2] = _grain_centers[i](1);
    _grain_volume_center_vector[4 * i + 3] = _grain_centers[i](2);
  }
}
