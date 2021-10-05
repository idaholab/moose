//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatchedValueBC.h"

registerMooseObject("MooseApp", MatchedValueBC);

defineLegacyParams(MatchedValueBC);

InputParameters
MatchedValueBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addRequiredCoupledVar("v", "The variable whose value we are to match.");
  params.addParam<Real>("u_coeff", 1.0, " A coefficient for primary variable u");
  params.addParam<Real>("v_coeff", 1.0, " A coefficient for coupled variable v");
  params.addClassDescription("Implements a NodalBC which equates two different Variables' values "
                             "on a specified boundary.");
  return params;
}

MatchedValueBC::MatchedValueBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _v(coupledValue("v")),
    _v_num(coupled("v")),
    _u_coeff(getParam<Real>("u_coeff")),
    _v_coeff(getParam<Real>("v_coeff"))
{
}

Real
MatchedValueBC::computeQpResidual()
{
  return _u_coeff * _u[_qp] - _v_coeff * _v[_qp];
}

Real
MatchedValueBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_num)
    return -1.0 * _v_coeff;
  else
    return 0.;
}
