//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatReaction.h"

registerMooseObject("MooseApp", ADMatReaction);

InputParameters
ADMatReaction::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription(
      "Kernel representing the contribution of the PDE term $m*u$, where $m$ is a "
      "material property coefficient/reaction rate and $u$ is a scalar variable, and whose "
      "Jacobian contribution is calculated using sutomatic differentiation.");
  params.addParam<MaterialPropertyName>(
      "mat_prop_name", 1.0, "The coefficient / reaction rate used with the kernel.");
  return params;
}

ADMatReaction::ADMatReaction(const InputParameters & parameters)
  : ADKernel(parameters), _mat_prop(getADMaterialProperty<Real>("mat_prop_name"))
{
}

ADReal
ADMatReaction::computeQpResidual()
{
  return _mat_prop[_qp] * _test[_i][_qp] * _u[_qp];
}
