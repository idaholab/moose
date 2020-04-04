//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainCentersPostprocessor.h"
#include "ComputeGrainCenterUserObject.h"

InputParameters
GrainCentersPostprocessor::validParams()
{
  InputParameters params = VectorPostprocessor::validParams();
  params.addClassDescription("Outputs the values from ComputeGrainCenterUserObject");
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
