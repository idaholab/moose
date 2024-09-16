//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSolidPropertiesFunctorMaterial.h"
#include "ThermalSolidProperties.h"
#include "SolidPropertiesNames.h"

registerMooseObject("SolidPropertiesApp", ThermalSolidPropertiesFunctorMaterial);

InputParameters
ThermalSolidPropertiesFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addRequiredParam<MooseFunctorName>("temperature", "Temperature");
  params.addRequiredParam<UserObjectName>("sp", "The name of the user object for solid properties");
  params.addParam<bool>("use_constant_density", false, "Use constant density evaluated at 'T_ref'");
  params.addParam<Real>(
      "T_ref",
      "Temperature at which to evaluate density if 'use_constant_density' is set to 'true'");
  params.addParam<std::string>(SolidPropertiesNames::specific_heat,
                               SolidPropertiesNames::specific_heat,
                               "Name to be used for the isobaric specific heat");
  params.addParam<std::string>(SolidPropertiesNames::thermal_conductivity,
                               SolidPropertiesNames::thermal_conductivity,
                               "Name to be used for the thermal conductivity");
  params.addParam<std::string>(SolidPropertiesNames::density,
                               SolidPropertiesNames::density,
                               "Name to be used for the density");
  params.addParam<std::string>(SolidPropertiesNames::specific_internal_energy,
                               SolidPropertiesNames::specific_internal_energy,
                               "Name to be used for the specific internal energy");
  params.addClassDescription("Computes solid thermal properties as a function of temperature");
  return params;
}

ThermalSolidPropertiesFunctorMaterial::ThermalSolidPropertiesFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _temperature(getFunctor<ADReal>("temperature")),
    _sp(getUserObject<ThermalSolidProperties>("sp"))
{
  if (getParam<bool>("use_constant_density"))
  {
    if (isParamValid("T_ref"))
    {
      const Real T_ref = getParam<Real>("T_ref");
      addFunctorProperty<ADReal>(getParam<std::string>(SolidPropertiesNames::density),
                                 [this, T_ref](const auto & /*r*/, const auto & /*t*/) -> ADReal
                                 { return _sp.rho_from_T(T_ref); });
    }
    else
      paramError("T_ref",
                 "The parameter 'T_ref' is required if 'use_constant_density' is set to 'true'.");
  }
  else
  {
    if (isParamValid("T_ref"))
      paramError("T_ref",
                 "The parameter 'T_ref' may not be specified if 'use_constant_density' is set to "
                 "'false'.");
    else
      addFunctorProperty<ADReal>(getParam<std::string>(SolidPropertiesNames::density),
                                 [this](const auto & r, const auto & t) -> ADReal
                                 { return _sp.rho_from_T(_temperature(r, t)); });
  }

  addFunctorProperty<ADReal>(getParam<std::string>(SolidPropertiesNames::specific_heat),
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _sp.cp_from_T(_temperature(r, t)); });
  addFunctorProperty<ADReal>(getParam<std::string>(SolidPropertiesNames::thermal_conductivity),
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _sp.k_from_T(_temperature(r, t)); });
  addFunctorProperty<ADReal>(getParam<std::string>(SolidPropertiesNames::specific_internal_energy),
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _sp.e_from_T(_temperature(r, t)); });
}
