//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DotCouplingKernel.h"

registerMooseObject("MooseTestApp", DotCouplingKernel);

InputParameters
DotCouplingKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addCoupledVar("v", "Variable being coupled");
  return params;
}

DotCouplingKernel::DotCouplingKernel(const InputParameters & parameters)
  : Kernel(parameters), _v_dot(coupledDot("v")), _dv_dot_dv(coupledDotDu("v"))
{
}

Real
DotCouplingKernel::computeQpResidual()
{
  return -_v_dot[_qp] * _test[_i][_qp];
}

Real
DotCouplingKernel::computeQpJacobian()
{
  return -_dv_dot_dv[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
