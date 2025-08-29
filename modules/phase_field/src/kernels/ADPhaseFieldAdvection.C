//* This file is part of the MOOSE framework
//* https://www.mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPhaseFieldAdvection.h"

registerMooseObject("PhaseFieldApp", ADPhaseFieldAdvection);

InputParameters
ADPhaseFieldAdvection::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription("Implements the phasefield advection equation: $\\vec{v}\\cdot\\nabla "
                             "u = 0$, where the weak form is $(\\psi_i, \\vec{v}\\cdot\\nabla u) = "
                             "0$.");
  params.addRequiredCoupledVar("velocity", "Velocity vector variable.");
  return params;
}

ADPhaseFieldAdvection::ADPhaseFieldAdvection(const InputParameters & parameters)
  : ADKernelValue(parameters), _velocity(adCoupledVectorValue("velocity"))
{
}

ADReal
ADPhaseFieldAdvection::precomputeQpResidual()
{
  return _velocity[_qp] * _grad_u[_qp];
}
