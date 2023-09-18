//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldAdvection.h"

registerMooseObject("PhaseFieldApp", PhaseFieldAdvection);

InputParameters
PhaseFieldAdvection::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription("Implements the phasefield advection equation: $\\vec{v}\\cdot\\nabla "
                             "u = 0$, where the weak form is $(\\psi_i, \\vec{v}\\cdot\\nabla u) = "
                             "0$.");
  params.addRequiredCoupledVar("velocity", "Velocity vector variable.");
  return params;
}

PhaseFieldAdvection::PhaseFieldAdvection(const InputParameters & parameters)
  : ADKernelValue(parameters), _velocity(adCoupledVectorValue("velocity"))
{
}

ADReal
PhaseFieldAdvection::precomputeQpResidual()
{
  return _velocity[_qp] * _grad_u[_qp];
}
