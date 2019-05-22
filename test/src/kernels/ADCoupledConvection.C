//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledConvection.h"

registerADMooseObject("MooseTestApp", ADCoupledConvection);

defineADValidParams(ADCoupledConvection,
                    ADKernel,
                    params.addParam<Real>("scale", 1, "Scaling coefficient");
                    params.addRequiredCoupledVar("velocity_vector",
                                                 "Velocity Vector for the Convection ADKernel"););

template <ComputeStage compute_stage>
ADCoupledConvection<compute_stage>::ADCoupledConvection(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters),
    _velocity_vector(adCoupledGradient("velocity_vector")),
    _scale(adGetParam<Real>("scale"))
{
}

template <ComputeStage compute_stage>
ADResidual
ADCoupledConvection<compute_stage>::computeQpResidual()
{
  return _scale * _test[_i][_qp] * _velocity_vector[_qp] * _grad_u[_qp];
}
