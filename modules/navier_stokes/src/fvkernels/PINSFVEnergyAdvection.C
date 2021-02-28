//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVEnergyAdvection.h"
#include "PINSFVVelocityVariable.h"

registerMooseObject("NavierStokesApp", PINSFVEnergyAdvection);

InputParameters
PINSFVEnergyAdvection::validParams()
{
  auto params = INSFVMomentumAdvection::validParams();
  params.addClassDescription("Advects energy, e.g. rho*cp*T. A user may still override what "
                             "quantity is advected, but the default is rho*cp*T");
  params.set<MaterialPropertyName>("advected_quantity") = "rho_cp_temp";
  return params;
}

PINSFVEnergyAdvection::PINSFVEnergyAdvection(const InputParameters & params)
  : INSFVMomentumAdvection(params)
{
  if (!dynamic_cast<const PINSFVVelocityVariable *>(_u_var))
    mooseError("PINSFVEnergyAdvection may only be used with a superficial advective velocity, "
        "of variable type PINSFVVelocityVariable.");
}
