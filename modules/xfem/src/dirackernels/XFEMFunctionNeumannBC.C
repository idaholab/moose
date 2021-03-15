//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMFunctionNeumannBC.h"

registerMooseObject("XFEMApp", XFEMFunctionNeumannBC);

InputParameters
XFEMFunctionNeumannBC::validParams()
{
  InputParameters params = XFEMIntegratedBC::validParams();
  params.addClassDescription("A constant Neumann BC on an XFEM interface.");
  params.addRequiredParam<FunctionName>("function", "The function");
  return params;
}

XFEMFunctionNeumannBC::XFEMFunctionNeumannBC(const InputParameters & parameters)
  : XFEMIntegratedBC(parameters), _func(getFunction("function"))
{
}

Real
XFEMFunctionNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * _func.value(_t, _current_point);
}

Real
XFEMFunctionNeumannBC::computeQpJacobian()
{
  return 0;
}
