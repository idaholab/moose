//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionVariableIntegral.h"

#include "Assembly.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"

#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

registerMooseObject("isopodApp", DiffusionVariableIntegral);

defineLegacyParams(DiffusionVariableIntegral);

InputParameters
DiffusionVariableIntegral::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addRequiredCoupledVar("variable_forward", "forward solution");
  return params;
}

DiffusionVariableIntegral::DiffusionVariableIntegral(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
    _grad_u_forward(coupledGradient("variable_forward"))
{
}

Real
DiffusionVariableIntegral::getValue()
{
  return ElementIntegralPostprocessor::getValue();
}

Real
DiffusionVariableIntegral::computeQpIntegral()
{
  return - _grad_u[_qp] * _grad_u_forward[_qp];
}
