//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeRotatedElasticityTensorBase.h"
#include "RotationTensor.h"

template <bool is_ad>
InputParameters
ComputeRotatedElasticityTensorBaseTempl<is_ad>::validParams()
{
  InputParameters params = ComputeElasticityTensorBaseTempl<is_ad>::validParams();
  params.addParam<Real>("euler_angle_1", 0.0, "Euler angle in direction 1");
  params.addParam<Real>("euler_angle_2", 0.0, "Euler angle in direction 2");
  params.addParam<Real>("euler_angle_3", 0.0, "Euler angle in direction 3");
  params.addParam<RealTensorValue>("rotation_matrix",
                                   "Rotation matrix to apply to elasticity tensor.");
  return params;
}

template <bool is_ad>
ComputeRotatedElasticityTensorBaseTempl<is_ad>::ComputeRotatedElasticityTensorBaseTempl(
    const InputParameters & parameters)
  : ComputeElasticityTensorBaseTempl<is_ad>(parameters),
    _Euler_angles(this->template getParam<Real>("euler_angle_1"),
                  this->template getParam<Real>("euler_angle_2"),
                  this->template getParam<Real>("euler_angle_3")),
    _rotation_matrix(this->isParamValid("rotation_matrix")
                         ? this->template getParam<RealTensorValue>("rotation_matrix")
                         : RealTensorValue(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0))
{
}

template class ComputeRotatedElasticityTensorBaseTempl<false>;
template class ComputeRotatedElasticityTensorBaseTempl<true>;
