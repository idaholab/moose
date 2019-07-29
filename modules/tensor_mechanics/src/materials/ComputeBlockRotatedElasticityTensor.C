
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeBlockRotatedElasticityTensor.h"
#include "EulerAngleProvider.h"
#include "RotationTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeBlockRotatedElasticityTensor);

template <>
InputParameters
validParams<ComputeBlockRotatedElasticityTensor>()
{
  InputParameters params = validParams<ComputeElasticityTensorBase>();
  params.addClassDescription("Compute an elasticity tensor.");
  params.addRequiredParam<std::vector<Real>>("C_ijkl", "Stiffness tensor for material");
  params.addParam<MooseEnum>(
      "fill_method", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  params.addRequiredParam<UserObjectName>("euler_angle_provider",
                                          "Name of Euler angle provider user object");
  params.addRequiredParam<int>("offset", "Offset value for grain_id");

  return params;
}

ComputeBlockRotatedElasticityTensor::ComputeBlockRotatedElasticityTensor(
    const InputParameters & parameters)
  : ComputeElasticityTensorBase(parameters),
    _C_ijkl(getParam<std::vector<Real>>("C_ijkl"),
            (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method")),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider")),
    _offset(getParam<int>("offset"))
{
}
void
ComputeBlockRotatedElasticityTensor::computeQpElasticityTensor()
{
  EulerAngles angles;
  auto grain_id = _current_elem->subdomain_id();
  if (grain_id == 0 && _offset != 0)
    mooseError("Please set correct offset value ");
  grain_id = grain_id - _offset;

  if (grain_id < _euler.getGrainNum())
    angles = _euler.getEulerAngles(grain_id);
  else
    mooseError(
        "invalid block id ", grain_id, " (only have ", _euler.getGrainNum(), " Eulerangles)");

  RankFourTensor C_ijkl = _C_ijkl;
  C_ijkl.rotate(RotationTensor(RealVectorValue(angles)));
  _elasticity_tensor[_qp] = C_ijkl;
}
