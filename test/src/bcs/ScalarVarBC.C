//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarVarBC.h"

registerMooseObject("MooseTestApp", ScalarVarBC);

InputParameters
ScalarVarBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredCoupledVar("alpha", "The scalar variable coupled in");
  return params;
}

ScalarVarBC::ScalarVarBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _alpha_var(coupledScalar("alpha")),
    _alpha(coupledScalarValue("alpha"))
{
}

Real
ScalarVarBC::computeQpResidual()
{
  return -_alpha[0] * _test[_i][_qp];
}

Real
ScalarVarBC::computeQpOffDiagJacobianScalar(unsigned int jvar)
{
  if (jvar == _alpha_var)
    return -_test[_i][_qp];
  else
    return 0.;
}
