//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledForceLagged.h"

registerMooseObject("MooseTestApp", CoupledForceLagged);

InputParameters
CoupledForceLagged::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar("v", "The coupled variable which provides the force");
  params.addParam<Real>("coefficient", 1.0, "Coefficient of the term");
  params.addParam<TagName>(
      "tag", Moose::PREVIOUS_NL_SOLUTION_TAG, "The solution vector to be coupled in");
  return params;
}

CoupledForceLagged::CoupledForceLagged(const InputParameters & parameters)
  : Kernel(parameters),
    _v_var(coupled("v")),
    _v(coupledVectorTagValue("v", "tag")),
    _coef(getParam<Real>("coefficient"))
{
}

Real
CoupledForceLagged::computeQpResidual()
{
  return -_coef * _v[_qp] * _test[_i][_qp];
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
