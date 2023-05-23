//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFEMomentumFreeSlipBC.h"

registerMooseObject("NavierStokesApp", INSFEMomentumFreeSlipBC);
registerMooseObjectRenamed("NavierStokesApp",
                           MDMomentumFreeSlipBC,
                           "02/01/2024 00:00",
                           INSFEMomentumFreeSlipBC);

InputParameters
INSFEMomentumFreeSlipBC::validParams()
{
  InputParameters params = MomentumFreeSlipBC::validParams();
  params.renameCoupledVar("rho_u", "u", "x-component of velocity");
  params.renameCoupledVar("rho_v", "v", "y-component of velocity");
  params.renameCoupledVar("rho_w", "w", "z-component of velocity");
  return params;
}

INSFEMomentumFreeSlipBC::INSFEMomentumFreeSlipBC(const InputParameters & parameters)
  : MomentumFreeSlipBC(parameters)
{
}
