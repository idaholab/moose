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
  _eps_neighbor(coupledNeighborValue("porosity"))
{
  if (!dynamic_cast<const PINSFVVelocityVariable *>(_u_var))
    mooseError("PINSFVMomentumAdvection may only be used with a superficial advective velocity, "
        "of variable type PINSFVVelocityVariable.");
}

ADReal
PINSFVMomentumAdvection::computeQpResidual()
{
  // TODO optimize to avoid interpolating velocity twice
  // ADRealVectorValue v_face;
  // this->interpolate(_velocity_interp_method, v_face, _vel_elem[_qp], _vel_neighbor[_qp]);

  // Compute face porosity gradient
  Real eps_face;
  Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                         eps_face,
                         _eps[_qp],
                         _eps_neighbor[_qp],
                         *_face_info,
                         true);

  ADReal residual = INSFVMomentumAdvection::computeQpResidual() / eps_face;

  return residual;
}
