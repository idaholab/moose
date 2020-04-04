//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsPPenalty.h"

#include <iostream>

registerMooseObject("RichardsApp", RichardsPPenalty);

InputParameters
RichardsPPenalty::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<Real>(
      "a",
      1.0E-10,
      "Weight of the penalty.  Penalty = a*(lower - variable) for variable<lower, "
      "and zero otherwise.  Care should be taken with this parameter choice.  "
      "Determine the typical size of your residual (usually rho*perm*(gradP - "
      "rho*g)/visc), then typically you want the penalty to ensure p>lower*(1-1E-6), "
      "so for the PPP formulation you typically Penalty = a*1E-6*|p|.  I recommend "
      "that Penalty = 1E-3*residual, yielding a = 1E3*residual/|P|. ");
  params.addRequiredCoupledVar(
      "lower_var", "Your variable will be constrained to be greater than this lower_var variable.");
  params.addClassDescription("This adds a term to the residual that attempts to enforce variable > "
                             "lower_var.  The term is a*(lower - variable) for variable<lower, and "
                             "zero otherwise");
  return params;
}

RichardsPPenalty::RichardsPPenalty(const InputParameters & parameters)
  : Kernel(parameters),
    _a(getParam<Real>("a")),
    _lower(coupledValue("lower_var")),
    _lower_var_num(coupled("lower_var"))
{
}

Real
RichardsPPenalty::computeQpResidual()
{
  if (_u[_qp] < _lower[_qp])
    return _test[_i][_qp] * _a * (_lower[_qp] - _u[_qp]);

  return 0.0;
}

Real
RichardsPPenalty::computeQpJacobian()
{
  if (_u[_qp] < _lower[_qp])
    return -_test[_i][_qp] * _a * _phi[_j][_qp];
  ;

  return 0.0;
}

Real
RichardsPPenalty::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _lower_var_num && _u[_qp] < _lower[_qp])
    return _test[_i][_qp] * _a * _phi[_j][_qp];

  return 0.0;
}
