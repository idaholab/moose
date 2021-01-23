//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumAdvection.h"
#include "PINSFVVelocityVariable.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumAdvection);

InputParameters
PINSFVMomentumAdvection::validParams()
{
  auto params = INSFVMomentumAdvection::validParams();
  params.addClassDescription("Object for advecting mass in porous media mass equation");
  params.addRequiredCoupledVar("porosity", "Porosity auxiliary variable");

  return params;
}

PINSFVMomentumAdvection::PINSFVMomentumAdvection(const InputParameters & params)
  : INSFVMomentumAdvection(params),
  _eps(coupledValue("porosity")),
  _grad_eps(coupledGradient("porosity"))
{
  if (!dynamic_cast<const PINSFVVelocityVariable *>(_u_var))
    mooseError("PINSFVMomentumAdvection may only be used with a superficial advective velocity, "
        "of variable type PINSFVVelocityVariable.");
}

ADReal
PINSFVMomentumAdvection::computeQpResidual()
{
  ADReal residual = INSFVMomentumAdvection::computeQpResidual() / _eps[_qp];

  /// Add porosity gradient term
  //FIXME Should be an ElementalKernel ?
  residual += _rho * ADRealVectorValue(_vel_elem[_qp](0) * _vel_elem[_qp](0),
      _vel_elem[_qp](1) * _vel_elem[_qp](1), _vel_elem[_qp](2) * _vel_elem[_qp](2))
   * (-_grad_eps[_qp] / _eps[_qp] / _eps[_qp]);

  return residual;
}
