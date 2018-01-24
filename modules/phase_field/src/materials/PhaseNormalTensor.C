/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PhaseNormalTensor.h"

template <>
InputParameters
validParams<PhaseNormalTensor>()
{
  InputParameters params = validParams<Material>();
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
    RealVectorValue vector = _grad_u[_qp] / magnitude;
    _normal_tensor[_qp].vectorOuterProduct(vector, vector);
  }
  else
    _normal_tensor[_qp].zero();
}
