//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvectedMeshPSPG.h"

registerMooseObject("FsiApp", ConvectedMeshPSPG);

InputParameters
ConvectedMeshPSPG::validParams()
{
  InputParameters params = INSBase::validParams();
  params.addClassDescription(
      "Corrects the convective derivative for situations in which the fluid mesh is dynamic.");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
  return params;
}

ConvectedMeshPSPG::ConvectedMeshPSPG(const InputParameters & parameters)
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
    _rho(getMaterialProperty<Real>("rho_name"))
{
}

RealVectorValue
ConvectedMeshPSPG::strongResidual()
{
  const auto minus_rho_ddisp_dt =
      -_rho[_qp] * RealVectorValue(_disp_x_dot[_qp], _disp_y_dot[_qp], _disp_z_dot[_qp]);
  return RealVectorValue(minus_rho_ddisp_dt * _grad_u_vel[_qp],
                         minus_rho_ddisp_dt * _grad_v_vel[_qp],
                         minus_rho_ddisp_dt * _grad_w_vel[_qp]);
}

RealVectorValue
ConvectedMeshPSPG::dStrongResidualDDisp(const unsigned short component)
{
  const auto & ddisp_dot = [&]() -> const VariableValue &
  {
    switch (component)
    {
      case 0:
        return _d_disp_x_dot;
      case 1:
        return _d_disp_y_dot;
      case 2:
        return _d_disp_z_dot;
      default:
        mooseError("Invalid component");
    }
  }();

  // Only non-zero component will be from 'component'
  RealVectorValue ddisp_dt;
  ddisp_dt(component) = _phi[_j][_qp] * ddisp_dot[_qp];

  const auto minus_rho_ddisp_dt = -_rho[_qp] * ddisp_dt;
  return RealVectorValue(minus_rho_ddisp_dt * _grad_u_vel[_qp],
                         minus_rho_ddisp_dt * _grad_v_vel[_qp],
                         minus_rho_ddisp_dt * _grad_w_vel[_qp]);
}

RealVectorValue
ConvectedMeshPSPG::dStrongResidualDVel(const unsigned short component)
{
  const auto minus_rho_ddisp_dt =
      -_rho[_qp] * RealVectorValue(_disp_x_dot[_qp], _disp_y_dot[_qp], _disp_z_dot[_qp]);

  // Only non-zero component will be from 'component'
  RealVectorValue ret;
  ret(component) = minus_rho_ddisp_dt * _grad_phi[_j][_qp];
  return ret;
}

Real
ConvectedMeshPSPG::computeQpResidual()
{
  return -tau() / _rho[_qp] * _grad_test[_i][_qp] * strongResidual();
}

Real
ConvectedMeshPSPG::computeQpJacobian()
{
  // No derivative with respect to pressure
  return 0;
}

Real
ConvectedMeshPSPG::computeQpOffDiagJacobian(unsigned int jvar)
{
  mooseAssert(jvar != _var.number(), "Making sure I understand how old hand-coded Jacobians work.");

  if (jvar == _disp_x_id)
    return -tau() / _rho[_qp] * _grad_test[_i][_qp] * dStrongResidualDDisp(0);
  else if (jvar == _disp_y_id)
    return -tau() / _rho[_qp] * _grad_test[_i][_qp] * dStrongResidualDDisp(1);
  else if (jvar == _disp_z_id)
    return -tau() / _rho[_qp] * _grad_test[_i][_qp] * dStrongResidualDDisp(2);
  else if (jvar == _u_vel_var_number)
    return -dTauDUComp(0) / _rho[_qp] * _grad_test[_i][_qp] * strongResidual() -
           tau() / _rho[_qp] * _grad_test[_i][_qp] * dStrongResidualDVel(0);
  else if (jvar == _v_vel_var_number)
    return -dTauDUComp(1) / _rho[_qp] * _grad_test[_i][_qp] * strongResidual() -
           tau() / _rho[_qp] * _grad_test[_i][_qp] * dStrongResidualDVel(1);
  else if (jvar == _w_vel_var_number)
    return -dTauDUComp(2) / _rho[_qp] * _grad_test[_i][_qp] * strongResidual() -
           tau() / _rho[_qp] * _grad_test[_i][_qp] * dStrongResidualDVel(2);
  else
    return 0.0;
}
