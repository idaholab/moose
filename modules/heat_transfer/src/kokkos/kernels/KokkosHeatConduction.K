//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosHeatConduction.h"

registerMooseObject("HeatTransferApp", KokkosHeatConduction);

InputParameters
KokkosHeatConduction::validParams()
{
  InputParameters params = KokkosDiffusion::validParams();
  params.addClassDescription(
      "Computes residual/Jacobian contribution for $(k \\nabla T, \\nabla \\psi)$ term.");
  params.addParam<MaterialPropertyName>(
      "diffusion_coefficient", "thermal_conductivity", "Property name of the diffusivity");
  params.addParam<MaterialPropertyName>(
      "diffusion_coefficient_dT",
      "thermal_conductivity_dT",
      "Property name of the derivative of the diffusivity with respect "
      "to the variable (Default: thermal_conductivity_dT)");
  // params.set<bool>("use_displaced_mesh") = true;
  return params;
}

KokkosHeatConduction::KokkosHeatConduction(const InputParameters & parameters)
  : KokkosDiffusion(parameters)
{
  _diffusion_coefficient = getKokkosMaterialProperty<Real>("diffusion_coefficient");

  if (hasKokkosMaterialProperty<Real>("diffusion_coefficient_dT"))
    _diffusion_coefficient_dT = getKokkosMaterialProperty<Real>("diffusion_coefficient_dT");
}
