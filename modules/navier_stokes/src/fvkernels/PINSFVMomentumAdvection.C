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
  // FIXME Add porosity in Rhie Chow interpolation
  ADRealVectorValue v;
  ADReal adv_quant_interface;

  this->interpolate(_velocity_interp_method, v, _vel_elem[_qp], _vel_neighbor[_qp]);
  Moose::FV::interpolate(_advected_interp_method,
                         adv_quant_interface,
                         _adv_quant_elem[_qp] / _eps[_qp],
                         _adv_quant_neighbor[_qp] / _eps_neighbor[_qp],
                         v,
                         *_face_info,
                         true);

  return _normal * v * adv_quant_interface;
}
