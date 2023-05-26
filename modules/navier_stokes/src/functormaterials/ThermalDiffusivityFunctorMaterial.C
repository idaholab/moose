//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "ThermalDiffusivityFunctorMaterial.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", ThermalDiffusivityFunctorMaterial);

InputParameters
ThermalDiffusivityFunctorMaterial::validParams()
{
  auto params = FunctorMaterial::validParams();
  params.addClassDescription("Computes the thermal diffusivity given the thermal conductivity, "
                             "specific heat capacity, and fluid density.");
  params.addRequiredParam<MooseFunctorName>(NS::k, "The thermal conductivity.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density.");
  params.addRequiredParam<MooseFunctorName>(NS::cp, "The specific heat capacity.");
  return params;
}

ThermalDiffusivityFunctorMaterial::ThermalDiffusivityFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _k(getFunctor<ADReal>(NS::k)),
    _cp(getFunctor<ADReal>(NS::cp)),
    _rho(getFunctor<ADReal>(NS::density))
{
  addFunctorProperty<ADReal>(NS::thermal_diffusivity,
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _k(r, t) / (_rho(r, t) * _cp(r, t)); });
}
