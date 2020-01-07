//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PMassKernel.h"

registerMooseObject("MooseTestApp", PMassKernel);

InputParameters
PMassKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRangeCheckedParam<Real>("p", 2.0, "p>=1.0", "The actual exponent is p-2");
  params.addParam<Real>("coefficient", 1.0, "Coefficient of the term");
  return params;
}

PMassKernel::PMassKernel(const InputParameters & parameters)
  : Kernel(parameters), _p(getParam<Real>("p") - 2.0), _coef(getParam<Real>("coefficient"))
{
}

Real
PMassKernel::computeQpResidual()
{
  return _coef * std::pow(std::fabs(_u[_qp]), _p) * _u[_qp] * _test[_i][_qp];
}

Real
PMassKernel::computeQpJacobian()
{
  // Note: this jacobian evaluation is not exact when p!=2.
  return _coef * std::pow(std::fabs(_phi[_j][_qp]), _p) * _phi[_j][_qp] * _test[_i][_qp];
}
