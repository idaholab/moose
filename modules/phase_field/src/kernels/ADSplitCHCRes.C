//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSplitCHCRes.h"

defineADValidParams(
    ADSplitCHCRes,
    ADSplitCHBase,
    params.addClassDescription("Split formulation Cahn-Hilliard Kernel");
    params.addRequiredCoupledVar("w", "Chemical potential variable");
    params.addRequiredParam<MaterialPropertyName>("kappa_name", "The kappa used with the kernel"););

template <ComputeStage compute_stage>
ADSplitCHCRes<compute_stage>::ADSplitCHCRes(const InputParameters & parameters)
  : ADSplitCHBase<compute_stage>(parameters),
    _kappa(adGetADMaterialProperty<Real>("kappa_name")),
    _w(adCoupledValue("w"))
{
}

template <ComputeStage compute_stage>
ADReal
ADSplitCHCRes<compute_stage>::computeQpResidual()
{
  auto residual = ADSplitCHBase<compute_stage>::computeQpResidual();

  residual += -_w[_qp] * _test[_i][_qp];
  residual += _kappa[_qp] * _grad_u[_qp] * _grad_test[_i][_qp];

  return residual;
}

// explicit instantiation is required for AD base classes
adBaseClass(ADSplitCHCRes);
