/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GBDependentDiffusivity.h"

template <>
InputParameters
validParams<GBDependentDiffusivity>()
{
  InputParameters params = validParams<GBDependentTensorBase>();
  params.addClassDescription("Compute diffusivity rank two tensor based on GB phase variable");
  return params;
}

GBDependentDiffusivity::GBDependentDiffusivity(const InputParameters & parameters)
  : GBDependentTensorBase(parameters)
{
}

void
GBDependentDiffusivity::initQpStatefulProperties()
{
  _gb_dependent_tensor[_qp].zero();
}

void
GBDependentDiffusivity::computeQpProperties()
{
  RankTwoTensor iden(RankTwoTensor::initIdentity);
  RankTwoTensor gb_tensor;

  gb_tensor = (1.0 - _gb[_qp]) * _bulk_parameter * iden +
              _gb[_qp] * _gb_parameter * (iden - _gb_normal_tensor[_qp]);
  gb_tensor.fillRealTensor(_gb_dependent_tensor[_qp]);
}
