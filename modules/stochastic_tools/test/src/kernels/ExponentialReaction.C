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

InputParameters
ExponentialReaction::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Implements a simple reaction term with the following weak form "
      "$(\\psi_i,\\frac{\\mu_1}{\\mu_2}\\left[e^{\\mu_2 u_h}-1\\right])$.");
  params.addParam<Real>("mu1", 0.3, "First coefficient in the nonlinear term.");
  params.addParam<Real>("mu2", 9, "Second coefficient in the nonlinear term.");
  params.declareControllable("mu1");
  params.declareControllable("mu2");
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
  return _test[_i][_qp] * _mu1 / _mu2 * (exp(_mu2 * _u[_qp]) - 1);
}

Real
ExponentialReaction::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp] * _mu1 * exp(_mu2 * _u[_qp]);
}
