//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldTimeDerivativeSUPG.h"

registerMooseObject("PhaseFieldApp", PhaseFieldTimeDerivativeSUPG);

InputParameters
PhaseFieldTimeDerivativeSUPG::validParams()
{
  InputParameters params = ADTimeKernelGrad::validParams();
  params.addClassDescription(
      "SUPG stablization terms for the time derivative of the level set equation.");
  params.addRequiredCoupledVar("velocity", "Velocity vector variable.");
  return params;
}

PhaseFieldTimeDerivativeSUPG::PhaseFieldTimeDerivativeSUPG(const InputParameters & parameters)
  : ADTimeKernelGrad(parameters), _velocity(adCoupledVectorValue("velocity"))
{
}

ADRealVectorValue
PhaseFieldTimeDerivativeSUPG::precomputeQpResidual()
{
  ADReal tau;
  if (_velocity[_qp].norm() > 1.0e-10)
    tau = _current_elem->hmin() / (2 * _velocity[_qp].norm());
  else
    tau = 0.0;
  return tau * _velocity[_qp] * _u_dot[_qp];
}
