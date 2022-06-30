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
      "Kernel representing the contribution of the PDE term $-L*v$, where $L$ is a reaction rate "
      "material property, $v$ is a scalar variable (nonlinear or coupled), and whose Jacobian "
      "contribution is calculated using automatic differentiation.");
  params.addCoupledVar("v",
                       "Set this to make v a coupled variable, otherwise it will use the "
                       "kernel's nonlinear variable for v");
  params.addParam<MaterialPropertyName>(
      "reaction_rate", "L", "The reaction_rate used with the kernel.");
  return params;
}

ADMatReaction::ADMatReaction(const InputParameters & parameters)
  : ADKernel(parameters),
    _v(isCoupled("v") ? adCoupledValue("v") : _u),
    _reaction_rate(getADMaterialProperty<Real>("reaction_rate"))
{
}

ADReal
ADMatReaction::computeQpResidual()
{
  return -_reaction_rate[_qp] * _test[_i][_qp] * _v[_qp];
}
