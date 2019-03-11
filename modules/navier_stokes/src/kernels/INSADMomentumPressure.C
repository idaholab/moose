//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumPressure.h"

registerADMooseObject("NavierStokesApp", INSADMomentumPressure);

defineADValidParams(
    INSADMomentumPressure,
    ADVectorKernel,
    params.addClassDescription("Adds the pressure term to the INS momentum equation");
    params.addRequiredCoupledVar("p", "The pressure");
    params.addParam<bool>("integrate_p_by_parts",
                          true,
                          "Whether to integrate the pressure term by parts"););

template <ComputeStage compute_stage>
INSADMomentumPressure<compute_stage>::INSADMomentumPressure(const InputParameters & parameters)
  : ADVectorKernel<compute_stage>(parameters),
    _integrate_p_by_parts(adGetParam<bool>("integrate_p_by_parts")),
    _p(adCoupledValue("p")),
    _grad_p(adCoupledGradient("p"))
{
}

template <ComputeStage compute_stage>
ADResidual
INSADMomentumPressure<compute_stage>::computeQpResidual()
{
  if (_integrate_p_by_parts)
    return -_p[_qp] * _grad_test[_i][_qp].tr();
  else
    return _test[_i][_qp] * _grad_p[_qp];
}
