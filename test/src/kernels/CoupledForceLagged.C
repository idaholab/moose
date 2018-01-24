//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledForceLagged.h"

template <>
InputParameters
validParams<CoupledForceLagged>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("v", "The coupled variable which provides the force");
  return params;
}

CoupledForceLagged::CoupledForceLagged(const InputParameters & parameters)
  : Kernel(parameters), _v_var(coupled("v")), _v(coupledValuePreviousNL("v"))
{
}

Real
CoupledForceLagged::computeQpResidual()
{
  return -_v[_qp] * _test[_i][_qp];
}

Real
CoupledForceLagged::computeQpJacobian()
{
  return 0;
}

Real
CoupledForceLagged::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  // No off-diagonal contribution, becuase v is lagged in newton iterate
  return 0.0;
}
