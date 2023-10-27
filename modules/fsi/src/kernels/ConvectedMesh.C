//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvectedMesh.h"

registerMooseObject("FsiApp", ConvectedMesh);

InputParameters
ConvectedMesh::validParams()
{
  InputParameters params = INSBase::validParams();
  params.addClassDescription(
      "Corrects the convective derivative for situations in which the fluid mesh is dynamic.");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
  params.addParam<bool>(
      "supg", false, "Whether to perform SUPG stabilization of the momentum residuals");
  return params;
}

ConvectedMesh::ConvectedMesh(const InputParameters & parameters)
  : INSBase(parameters),
    _disp_x_dot(coupledDot("disp_x")),
    _d_disp_x_dot(coupledDotDu("disp_x")),
    _disp_x_id(coupled("disp_x")),
    _disp_y_dot(isCoupled("disp_y") ? coupledDot("disp_y") : _zero),
    _d_disp_y_dot(isCoupled("disp_y") ? coupledDotDu("disp_y") : _zero),
    _disp_y_id(coupled("disp_y")),
    _disp_z_dot(isCoupled("disp_z") ? coupledDot("disp_z") : _zero),
    _d_disp_z_dot(isCoupled("disp_z") ? coupledDotDu("disp_z") : _zero),
    _disp_z_id(coupled("disp_z")),
    _rho(getMaterialProperty<Real>("rho_name")),
    _supg(getParam<bool>("supg"))
{
  if (_var.number() == _u_vel_var_number)
    _component = 0;
  else if (_var.number() == _v_vel_var_number)
    _component = 1;
  else if (_var.number() == _w_vel_var_number)
    _component = 2;
  else
    paramError("variable", "The variable must match one of the velocity variables.");
}

Real
ConvectedMesh::strongResidual()
{
  return -_rho[_qp] * RealVectorValue(_disp_x_dot[_qp], _disp_y_dot[_qp], _disp_z_dot[_qp]) *
         _grad_u[_qp];
}

Real
ConvectedMesh::computeQpResidual()
{
  auto test = _test[_i][_qp];
  const auto U = relativeVelocity();
  if (_supg)
    test += tau() * _grad_test[_i][_qp] * U;
  return test * strongResidual();
}

Real
ConvectedMesh::computePGVelocityJacobian(const unsigned short component)
{
  const auto U = relativeVelocity();
  return strongResidual() * ((dTauDUComp(component) * _grad_test[_i][_qp] * U) +
                             (tau() * _grad_test[_i][_qp](component) * _phi[_j][_qp]));
}

Real
ConvectedMesh::computeQpJacobian()
{
  auto test = _test[_i][_qp];
  const auto U = relativeVelocity();
  if (_supg)
    test += tau() * _grad_test[_i][_qp] * U;
  auto jac = test * -_rho[_qp] *
             RealVectorValue(_disp_x_dot[_qp], _disp_y_dot[_qp], _disp_z_dot[_qp]) *
             _grad_phi[_j][_qp];
  if (_supg)
    jac += computePGVelocityJacobian(_component);

  return jac;
}

Real
ConvectedMesh::computeQpOffDiagJacobian(unsigned int jvar)
{
  mooseAssert(jvar != _var.number(), "Making sure I understand how old hand-coded Jacobians work.");

  auto test = _test[_i][_qp];
  const auto U = relativeVelocity();
  if (_supg)
    test += tau() * _grad_test[_i][_qp] * U;

  if (jvar == _disp_x_id)
    return test * -_rho[_qp] * _phi[_j][_qp] * _d_disp_x_dot[_qp] * _grad_u[_qp](0);
  else if (jvar == _disp_y_id)
    return test * -_rho[_qp] * _phi[_j][_qp] * _d_disp_y_dot[_qp] * _grad_u[_qp](1);
  else if (jvar == _disp_z_id)
    return test * -_rho[_qp] * _phi[_j][_qp] * _d_disp_z_dot[_qp] * _grad_u[_qp](2);
  else if (jvar == _u_vel_var_number)
  {
    if (_supg)
      return computePGVelocityJacobian(0);
    else
      return 0;
  }
  else if (jvar == _v_vel_var_number)
  {
    if (_supg)
      return computePGVelocityJacobian(1);
    else
      return 0;
  }
  else if (jvar == _w_vel_var_number)
  {
    if (_supg)
      return computePGVelocityJacobian(2);
    else
      return 0;
  }
  else
    return 0.0;
}
