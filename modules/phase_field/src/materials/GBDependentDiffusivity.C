//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GBDependentDiffusivity.h"

registerMooseObject("PhaseFieldApp", GBDependentDiffusivity);

InputParameters
GBDependentDiffusivity::validParams()
{
  InputParameters params = GBDependentTensorBase::validParams();
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
