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
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Corrects the convective derivative for situations in which the fluid mesh is dynamic.");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
  return params;
}

ConvectedMesh::ConvectedMesh(const InputParameters & parameters)
  : Kernel(parameters),
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

Real
ConvectedMesh::computeQpResidual()
{
  return _test[_i][_qp] * -_rho[_qp] *
         RealVectorValue(_disp_x_dot[_qp], _disp_y_dot[_qp], _disp_z_dot[_qp]) * _grad_u[_qp];
}

Real
ConvectedMesh::computeQpJacobian()
{
  return _test[_i][_qp] * -_rho[_qp] *
         RealVectorValue(_disp_x_dot[_qp], _disp_y_dot[_qp], _disp_z_dot[_qp]) * _grad_phi[_j][_qp];
}

Real
ConvectedMesh::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _disp_x_id)
    return _test[_i][_qp] * -_rho[_qp] * _phi[_j][_qp] * _d_disp_x_dot[_qp] * _grad_u[_qp](0);
  else if (jvar == _disp_y_id)
    return _test[_i][_qp] * -_rho[_qp] * _phi[_j][_qp] * _d_disp_y_dot[_qp] * _grad_u[_qp](1);
  else if (jvar == _disp_z_id)
    return _test[_i][_qp] * -_rho[_qp] * _phi[_j][_qp] * _d_disp_z_dot[_qp] * _grad_u[_qp](2);
  else
    return 0.0;
}
