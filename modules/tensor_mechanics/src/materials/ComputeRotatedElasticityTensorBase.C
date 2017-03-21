/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeRotatedElasticityTensorBase.h"
#include "RotationTensor.h"

template <>
InputParameters
validParams<ComputeRotatedElasticityTensorBase>()
{
  InputParameters params = validParams<ComputeElasticityTensorBase>();
  params.addParam<Real>("euler_angle_1", 0.0, "Euler angle in direction 1");
  params.addParam<Real>("euler_angle_2", 0.0, "Euler angle in direction 2");
  params.addParam<Real>("euler_angle_3", 0.0, "Euler angle in direction 3");
  return params;
}

ComputeRotatedElasticityTensorBase::ComputeRotatedElasticityTensorBase(
    const InputParameters & parameters)
  : ComputeElasticityTensorBase(parameters),
    _Euler_angles(getParam<Real>("euler_angle_1"),
                  getParam<Real>("euler_angle_2"),
                  getParam<Real>("euler_angle_3"))
{
}
