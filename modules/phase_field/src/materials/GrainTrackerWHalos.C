//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainTrackerWHalos.h"
#include "GrainTrackerInterface.h"

registerMooseObject("PhaseFieldApp", GrainTrackerWHalos);

template <>
InputParameters
validParams<GrainTrackerWHalos>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVarWithAutoBuild(
      "v","var_name_base","op_num","Array of coupled variables");
  params.addRequiredParam<UserObjectName>("euler_angle_provider",
                                          "Name of the euler angle provider user object");
  params.addParam<UserObjectName>("grain_tracker_name", "grain_tracker",
                                          "The GrainTracker UserObject to get values from.");
  params.addParam<MaterialPropertyName>("f_name",
                                        "op_to_grain",
                                        "Gives the grain associated with order parameter according to location");
  return params;
}

GrainTrackerWHalos::GrainTrackerWHalos(const InputParameters & parameters)
  : Material(parameters),
    _op_num(coupledComponents("v")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker_name")),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider")),
    _F_name(getParam<MaterialPropertyName>("f_name"))
{
  _grain_num = _grain_tracker.getNumberActiveGrains();
  for(unsigned int i = 0; i < _op_num; ++i)
  {
    _qp_orientation_matrix.push_back(&declareProperty<RankTwoTensor>(_F_name + std::to_string(i)));
  }
}

void
GrainTrackerWHalos::computeQpProperties()
{
  for (unsigned int grain = 0; grain < _grain_num; ++grain)
    _orientation_matrix[grain] = RotationTensor(_euler.getEulerAngles(grain));

  _op_to_grain = _grain_tracker.getVarToFeatureVector(_current_elem->id());


  for(unsigned int i = 0; i < _op_num; ++i)
  {
    if(_op_to_grain[i] != GrainTracker::invalid_id)
      (*_qp_orientation_matrix[i])[_qp] = _orientation_matrix[_op_to_grain[i]];
    else
      for(int j = 0;j < 3; ++j)
        for(int k = 0; k < 3; ++k)
          (*_qp_orientation_matrix[i])[_qp](i,j) = 2;
  }

}
