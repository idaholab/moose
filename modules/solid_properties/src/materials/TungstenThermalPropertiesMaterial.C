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

InputParameters
TungstenThermalPropertiesMaterial::validParams()
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

TungstenThermalPropertiesMaterial::TungstenThermalPropertiesMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _temperature(coupledValue("temperature")),
    _k(declareProperty<Real>(
        getParam<std::string>(SolidPropertiesNames::thermal_conductivity))),
    _c_p(declareProperty<Real>(
        getParam<std::string>(SolidPropertiesNames::specific_heat))),
    _rho(declareProperty<Real>(
        getParam<std::string>(SolidPropertiesNames::density)))
{}

void
TungstenThermalPropertiesMaterial::computeQpProperties()
{
    if (_temperature[_qp] < 11 || _temperature[_qp] > 3600)
        flagInvalidSolution("Thermal properties out of bounds for the Tungsten temperature computation. Temperature has to be between 11K and 3600K.");

    _k[_qp] = (_temperature[_qp] < 55) ? _kA0 * std::pow(_temperature[_qp] / 1000, 8.740e-01) / (1 + _kA1 * _temperature[_qp] / 1000 + _kA2 * Utility::pow<2>(_temperature[_qp] / 1000) + _kA3 * Utility::pow<3>(_temperature[_qp] / 1000))
                                       : (_kB0 + _kB1 * _temperature[_qp] / 1000 + _kB2 * Utility::pow<2>(_temperature[_qp] / 1000) + _kB3 * Utility::pow<3>(_temperature[_qp] / 1000))/(_kC0 + _kC1 * _temperature[_qp] / 1000+ Utility::pow<2>(_temperature[_qp] / 1000));

    _c_p[_qp] = (_temperature[_qp] <= 293) ? _cA0 * std::pow(_temperature[_qp] / 1000, 3.030) / (1 + _cA1 * _temperature[_qp] / 1000 + _cA2 * Utility::pow<2>(_temperature[_qp] / 1000) + _cA3 * Utility::pow<3>(_temperature[_qp] / 1000))
                                           : _cB0 + _cB1 * _temperature[_qp] / 1000 + _cB2 * Utility::pow<2>(_temperature[_qp] / 1000) + _cB3 * Utility::pow<3>(_temperature[_qp] / 1000) + _cB_2 / Utility::pow<2>(_temperature[_qp] / 1000);

    _rho[_qp] = (_temperature[_qp] <= 294) ? _rA0 / Utility::pow<3>(1 + (_rA1 + _rA2 * _temperature[_qp] / 1000 + _rA3 * Utility::pow<2>(_temperature[_qp] / 1000) + _rA4 * Utility::pow<3>(_temperature[_qp] / 1000)) / 100)
                                           : _rA0 / Utility::pow<3>(1 + (_rB0 + _rB1 * _temperature[_qp] / 1000 + _rB2 * Utility::pow<2>(_temperature[_qp] / 1000) + _rB3 * Utility::pow<3>(_temperature[_qp] / 1000)) / 100);
}
