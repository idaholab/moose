//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMNeumannBC.h"

registerMooseObject("XFEMApp", XFEMNeumannBC);

InputParameters
XFEMNeumannBC::validParams()
{
  InputParameters params = XFEMIntegratedBC::validParams();
  params.addClassDescription("A constant Neumann BC on an XFEM interface.");
  params.addRequiredParam<Real>("value", "Value of the BC");
  return params;
}

XFEMNeumannBC::XFEMNeumannBC(const InputParameters & parameters)
  : XFEMIntegratedBC(parameters), _v(getParam<Real>("value"))
{
}

Real
XFEMNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * _v;
}

Real
XFEMNeumannBC::computeQpJacobian()
{
  return 0;
}
