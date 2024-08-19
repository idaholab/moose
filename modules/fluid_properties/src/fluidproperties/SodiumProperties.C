//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SodiumProperties.h"

registerMooseObject("FluidPropertiesApp", SodiumProperties);

InputParameters
SodiumProperties::validParams()
{
  InputParameters params = FluidProperties::validParams();
  params.addClassDescription("Fluid properties for sodium");
  params.set<std::string>("fp_type") = "sodium-specific-fp";
  params.addParam<Real>(
      "thermal_conductivity",
      "Optional value for thermal conductivity that overrides interal calculations");
  params.addParam<Real>("specific_heat",
                        "Optional value for specific heat that overrides interal calculations");
  return params;
}

SodiumProperties::SodiumProperties(const InputParameters & parameters)
  : FluidProperties(parameters),
    _k(isParamValid("thermal_conductivity") ? getParam<Real>("thermal_conductivity") : 0.0),
    _cp(isParamValid("specific_heat") ? getParam<Real>("specific_heat") : 0.0)
{
}
