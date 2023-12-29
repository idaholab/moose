//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DashpotBC.h"

registerMooseObject("TensorMechanicsApp", DashpotBC);

InputParameters
DashpotBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<unsigned int>(
      "component", "The displacement component corresponding the variable this BC acts on.");
  params.addRequiredCoupledVar("disp_x", "Displacement in the x direction");
  params.addCoupledVar("disp_y", "Displacement in the y direction");
  params.addCoupledVar("disp_z", "Displacement in the z direction");

  params.addParam<Real>("coefficient", 1.0, "The viscosity coefficient");

  return params;
}

DashpotBC::DashpotBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _component(getParam<unsigned int>("component")),
    _coefficient(getParam<Real>("coefficient")),
    _disp_x_var(coupled("disp_x")),
    _disp_y_var(isCoupled("disp_y") ? coupled("disp_y") : 0),
    _disp_z_var(isCoupled("disp_z") ? coupled("disp_z") : 0),

    _disp_x_dot(coupledDot("disp_x")),
    _disp_y_dot(isCoupled("disp_y") ? coupledDot("disp_y") : _zero),
    _disp_z_dot(isCoupled("disp_z") ? coupledDot("disp_z") : _zero)
{
}

Real
DashpotBC::computeQpResidual()
{
  RealVectorValue velocity(_disp_x_dot[_qp], _disp_y_dot[_qp], _disp_z_dot[_qp]);

  return _test[_i][_qp] * _coefficient * _normals[_qp] * velocity;
}

Real
DashpotBC::computeQpJacobian()
{
  RealVectorValue velocity;
  velocity(_component) = _phi[_j][_qp] / _dt;

  return _test[_i][_qp] * _coefficient * _normals[_qp] * velocity;
}

Real
DashpotBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  RealVectorValue velocity;
  unsigned int component = 0;

  if (jvar == _disp_x_var)
    component = 0;
  else if (jvar == _disp_y_var)
    component = 1;
  else if (jvar == _disp_z_var)
    component = 2;

  velocity(component) = _phi[_j][_qp] / _dt;

  return -_test[_i][_qp] * _normals[_qp] * velocity;
}
