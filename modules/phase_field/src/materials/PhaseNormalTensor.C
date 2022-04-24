//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseNormalTensor.h"

registerMooseObject("PhaseFieldApp", PhaseNormalTensor);

InputParameters
PhaseNormalTensor::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Calculate normal tensor of a phase based on gradient");
  params.addRequiredCoupledVar("phase", "Phase variable");
  params.addRequiredParam<MaterialPropertyName>("normal_tensor_name", "Name of normal tensor");
  return params;
}

PhaseNormalTensor::PhaseNormalTensor(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _grad_u(coupledGradient("phase")),
    _normal_tensor(
        declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("normal_tensor_name")))
{
}

void
PhaseNormalTensor::initQpStatefulProperties()
{
  _normal_tensor[_qp].zero();
}

void
PhaseNormalTensor::computeQpProperties()
{
  const Real magnitude = _grad_u[_qp].norm();

  if (magnitude > 0.0)
  {
    const RealVectorValue vector = _grad_u[_qp] / magnitude;
    _normal_tensor[_qp] = RankTwoTensor::selfOuterProduct(vector);
  }
  else
    _normal_tensor[_qp].zero();
}
