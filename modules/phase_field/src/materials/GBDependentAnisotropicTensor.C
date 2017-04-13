/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GBDependentAnisotropicTensor.h"

template <>
InputParameters
validParams<GBDependentAnisotropicTensor>()
{
  InputParameters params = validParams<GBDependentTensorBase>();
  params.addClassDescription("Compute anisotropic rank two tensor based on GB phase variable");
  return params;
}

GBDependentAnisotropicTensor::GBDependentAnisotropicTensor(const InputParameters & parameters)
  : GBDependentTensorBase(parameters)
{
}

void
GBDependentAnisotropicTensor::initQpStatefulProperties()
{
  _gb_dependent_tensor[_qp].zero();
}

void
GBDependentAnisotropicTensor::computeQpProperties()
{
  RankTwoTensor iden(RankTwoTensor::initIdentity);
  RankTwoTensor gb_tensor;

  gb_tensor =
      (1.0 - _gb[_qp]) * _bulk_parameter * iden + _gb[_qp] * _gb_parameter * _gb_normal_tensor[_qp];
  gb_tensor.fillRealTensor(_gb_dependent_tensor[_qp]);
}
