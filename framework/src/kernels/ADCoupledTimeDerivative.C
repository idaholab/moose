//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledTimeDerivative.h"

registerADMooseObject("MooseApp", ADCoupledTimeDerivative);

defineADLegacyParams(ADCoupledTimeDerivative);

template <ComputeStage compute_stage>
InputParameters
ADCoupledTimeDerivative<compute_stage>::validParams()
{
  auto params = ADKernelValue<compute_stage>::validParams();
  params.addClassDescription("Time derivative Kernel that acts on a coupled variable. Weak form: "
                             "$(\\psi_i, \\frac{\\partial v_h}{\\partial t})$.");
  params.addRequiredCoupledVar("v", "Coupled variable");
  return params;
}

template <ComputeStage compute_stage>
ADCoupledTimeDerivative<compute_stage>::ADCoupledTimeDerivative(const InputParameters & parameters)
  : ADKernelValue<compute_stage>(parameters), _v_dot(adCoupledDot("v"))
{
}

template <ComputeStage compute_stage>
ADReal
ADCoupledTimeDerivative<compute_stage>::precomputeQpResidual()
{
  return _v_dot[_qp];
}
