//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledDirichletBC.h"

registerMooseObject("MooseTestApp", CoupledDirichletBC);

InputParameters
CoupledDirichletBC::validParams()
{
  InputParameters params = DirichletBC::validParams();
  params.addRequiredCoupledVar("v", "The coupled variable");
  return params;
}

CoupledDirichletBC::CoupledDirichletBC(const InputParameters & parameters)
  : DirichletBC(parameters), _v(coupledValue("v")), _v_num(coupled("v")), _c(1.0)
{
}

Real
CoupledDirichletBC::computeQpResidual()
{
  return _c * _u[_qp] + _u[_qp] * _u[_qp] + _v[_qp] * _v[_qp] - _value;
}

Real
CoupledDirichletBC::computeQpJacobian()
{
  return _c + 2. * _u[_qp];
}

Real
CoupledDirichletBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_num)
    return 2. * _v[_qp];
  else
    return 0.;
}
