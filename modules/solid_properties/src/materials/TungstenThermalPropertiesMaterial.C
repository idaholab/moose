//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/**
 * This material computes Tungsten thermal properties as a function of temperature.
 * Constants are taken from Milner, J. L., Karkos, P., & Bowers, J. J. (2024).
 * Space Nuclear Propulsion (SNP) Material Property Handbook (No. SNP-HDBK-0008).
 * National Aeronautics and Space Administration (NASA). https://ntrs.nasa.gov/citations/20240004217
 */

#include "TungstenThermalPropertiesMaterial.h"
#include "libmesh/utility.h"
#include <cmath>

registerMooseObject("SolidPropertiesApp", TungstenThermalPropertiesMaterial);
registerMooseObject("SolidPropertiesApp", ADTungstenThermalPropertiesMaterial);

template <bool is_ad>
InputParameters
TungstenThermalPropertiesMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("temperature", "Temperature");
  params.addParam<std::string>(SolidPropertiesNames::thermal_conductivity,
                               SolidPropertiesNames::thermal_conductivity,
                               "Name to be used for the thermal conductivity");
  params.addParam<std::string>(SolidPropertiesNames::specific_heat,
                               SolidPropertiesNames::specific_heat,
                               "Name to be used for the specific heat");
  params.addParam<std::string>(SolidPropertiesNames::density,
                               SolidPropertiesNames::density,
                               "Name to be used for the density");
  params.addClassDescription("Computes tungsten thermal properties as a function of temperature");
  return params;
}
template <bool is_ad>
TungstenThermalPropertiesMaterialTempl<is_ad>::TungstenThermalPropertiesMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _temperature(genericCoupledValue<is_ad>("temperature")),
    _k(declareGenericProperty<Real, is_ad>(
        getParam<std::string>(SolidPropertiesNames::thermal_conductivity))),
    _c_p(declareGenericProperty<Real, is_ad>(
        getParam<std::string>(SolidPropertiesNames::specific_heat))),
    _rho(declareGenericProperty<Real, is_ad>(getParam<std::string>(SolidPropertiesNames::density)))
{
}

template <bool is_ad>
void
TungstenThermalPropertiesMaterialTempl<is_ad>::computeQpProperties()
{
  if (_temperature[_qp] < 5 || _temperature[_qp] > 3600)
    flagInvalidSolution("The temperature is out of bounds to calculate tungsten density. "
                        "Temperature has to be between 5 K and 3600 K.");

  if (_temperature[_qp] < 1 || _temperature[_qp] > 3653)
    flagInvalidSolution("The temperature is out of bounds to calculate tungsten thermal "
                        "conductivity. Temperature has to be between 1 K and 3653 K.");

  if (_temperature[_qp] < 11 || _temperature[_qp] > 3700)
    flagInvalidSolution("The temperature is out of bounds to calculate tungsten specific "
                        "heat. Temperature has to be between 11 K and 3700 K.");

  const auto temperature_scaled = _temperature[_qp] / 1000;

  _k[_qp] = (_temperature[_qp] < 55)
                ? _kA0 * std::pow(temperature_scaled, 8.740e-01) /
                      (1 + _kA1 * temperature_scaled + _kA2 * Utility::pow<2>(temperature_scaled) +
                       _kA3 * Utility::pow<3>(temperature_scaled))
                : (_kB0 + _kB1 * temperature_scaled + _kB2 * Utility::pow<2>(temperature_scaled) +
                   _kB3 * Utility::pow<3>(temperature_scaled)) /
                      (_kC0 + _kC1 * temperature_scaled + Utility::pow<2>(temperature_scaled));

  _c_p[_qp] =
      (_temperature[_qp] <= 293)
          ? _cA0 * std::pow(temperature_scaled, 3.030) /
                (1 + _cA1 * temperature_scaled + _cA2 * Utility::pow<2>(temperature_scaled) +
                 _cA3 * Utility::pow<3>(temperature_scaled))
          : _cB0 + _cB1 * temperature_scaled + _cB2 * Utility::pow<2>(temperature_scaled) +
                _cB3 * Utility::pow<3>(temperature_scaled) +
                _cB_2 / Utility::pow<2>(temperature_scaled);

  _rho[_qp] = (_temperature[_qp] <= 294)
                  ? _rA0 / Utility::pow<3>(1 + (_rA1 + _rA2 * temperature_scaled +
                                                _rA3 * Utility::pow<2>(temperature_scaled) +
                                                _rA4 * Utility::pow<3>(temperature_scaled)) /
                                                   100)
                  : _rA0 / Utility::pow<3>(1 + (_rB0 + _rB1 * temperature_scaled +
                                                _rB2 * Utility::pow<2>(temperature_scaled) +
                                                _rB3 * Utility::pow<3>(temperature_scaled)) /
                                                   100);
}

template class TungstenThermalPropertiesMaterialTempl<false>;
template class TungstenThermalPropertiesMaterialTempl<true>;
