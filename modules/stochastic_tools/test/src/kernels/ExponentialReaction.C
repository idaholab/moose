//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExponentialReaction.h"

registerMooseObject("StochasticToolsTestApp", ExponentialReaction);

defineLegacyParams(ExponentialReaction);

InputParameters
ExponentialReaction::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Implements a simple consuming reaction term with weak form $(\\psi_i, u_h)$.");
  params.addParam<Real>("mu1", "First coefficient in the nonlinear term.");
  params.addParam<Real>("mu2", "Second coefficient in the nonlinear term.");
  return params;
}

ExponentialReaction::ExponentialReaction(const InputParameters & parameters)
  :
  Kernel(parameters),
  _mu1(getParam<Real>("mu1")),
  _mu2(getParam<Real>("mu2"))
{}

Real
ExponentialReaction::computeQpResidual()
{
  return _test[_i][_qp] * _mu1/_mu2 * (exp(_mu2 * _u[_qp]) - 1);
}

/*
Real
Reaction::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp];
}
*/
