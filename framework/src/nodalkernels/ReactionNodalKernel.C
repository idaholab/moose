//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReactionNodalKernel.h"

#include "Function.h"

registerMooseObject("MooseApp", ReactionNodalKernel);

InputParameters
ReactionNodalKernel::validParams()
{
  InputParameters params = NodalKernel::validParams();
  params.addClassDescription("Implements a simple consuming reaction term at nodes");
  params.addParam<Real>("coeff", 1., "A coefficient for multiplying the reaction term");
  return params;
}

ReactionNodalKernel::ReactionNodalKernel(const InputParameters & parameters)
  : NodalKernel(parameters), _coeff(getParam<Real>("coeff"))
{
}

Real
ReactionNodalKernel::computeQpResidual()
{
  return _coeff * _u[_qp];
}

Real
ReactionNodalKernel::computeQpJacobian()
{
  return _coeff;
}
