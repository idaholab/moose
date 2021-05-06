//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVEnergyAdvection.h"
#include "INSFVEnergyVariable.h"
#include "PINSFVSuperficialVelocityVariable.h"

registerMooseObject("NavierStokesApp", PINSFVEnergyAdvection);

InputParameters
PINSFVEnergyAdvection::validParams()
{
  auto params = PINSFVMomentumAdvection::validParams();
  params.addClassDescription("Advects energy, e.g. rho*cp*T. A user may still override what "
                             "quantity is advected, but the default is rho*cp*T");
  params.set<MaterialPropertyName>("advected_quantity") = "rho_cp_temp";
  return params;
}

PINSFVEnergyAdvection::PINSFVEnergyAdvection(const InputParameters & params)
  : PINSFVMomentumAdvection(params)
{
  if (!dynamic_cast<const PINSFVSuperficialVelocityVariable *>(_u_var))
    mooseError("PINSFVEnergyAdvection may only be used with a superficial advective velocity, "
               "of variable type PINSFVSuperficialVelocityVariable.");
  if (!dynamic_cast<INSFVEnergyVariable *>(&_var))
    mooseError("PINSFVEnergyAdvection may only be used with a fluid temperature variable, "
               "of variable type INSFVEnergyVariable.");
}

ADReal
PINSFVEnergyAdvection::computeQpResidual()
{
  ADRealVectorValue v;
  ADReal adv_quant_interface;

  // Velocity interpolation
  this->interpolate(_velocity_interp_method, v, _vel_elem[_qp], _vel_neighbor[_qp]);

  // Interpolation of advected quantity
  Moose::FV::interpolate(_advected_interp_method,
                         adv_quant_interface,
                         _adv_quant_elem[_qp],
                         _adv_quant_neighbor[_qp],
                         v,
                         *_face_info,
                         true);

  return _normal * v * adv_quant_interface;
}
