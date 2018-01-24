//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SecondDerivativeImplicitEuler.h"
#include "SubProblem.h"

template <>
InputParameters
validParams<SecondDerivativeImplicitEuler>()
{
  InputParameters params = validParams<TimeKernel>();
  return params;
}

SecondDerivativeImplicitEuler::SecondDerivativeImplicitEuler(const InputParameters & parameters)
  : TimeKernel(parameters), _u_old(valueOld()), _u_older(valueOlder())
{
}

Real
SecondDerivativeImplicitEuler::computeQpResidual()
{
  return _test[_i][_qp] * ((_u[_qp] - 2 * _u_old[_qp] + _u_older[_qp]) / (_dt * _dt));
}

Real
SecondDerivativeImplicitEuler::computeQpJacobian()
{
  return _test[_i][_qp] * (_phi[_j][_qp] / (_dt * _dt));
}
