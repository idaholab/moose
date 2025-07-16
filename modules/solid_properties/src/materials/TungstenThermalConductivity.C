//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TungstenThermalConductivity.h"
#include "libmesh/utility.h"
#include <cmath>

registerMooseObject("SolidPropertiesApp", TungstenThermalConductivity);

InputParameters
TungstenThermalConductivity::validParams()
{
    InputParameters params = Material::validParams();
    params.addRequiredCoupledVar("temperature", "Temperature");
    params.addParam<std::string>(SolidPropertiesNames::thermal_conductivity,
                                 SolidPropertiesNames::thermal_conductivity,
                                 "Name to be used for the thermal conductivity");
    params.addClassDescription("Computes solid thermal properties as a function of temperature");
    return params;
}

TungstenThermalConductivity::TungstenThermalConductivity(
    const InputParameters & parameters)
  : Material(parameters),
    _temperature(coupledValue("temperature")),
    _k(declareProperty<Real>(
        getParam<std::string>(SolidPropertiesNames::thermal_conductivity)))
{
}

void
TungstenThermalConductivity::computeQpProperties()
{
    if (_temperature[_qp] < 1 || _temperature[_qp] > 3653)
        flagInvalidSolution("Thermal conductivity out of bounds for the Tungsten temperature computation. Temperature has to be between 1K and 3653K.");
    if (_temperature[_qp] < 55)
        _k[_qp] = _kA0 * std::pow(_temperature[_qp] / 1000, 8.740e-01) / (1 + _kA1 * _temperature[_qp] / 1000 + _kA2 * Utility::pow<2>(_temperature[_qp] / 1000) + _kA3 * Utility::pow<3>(_temperature[_qp] / 1000));
    else
        _k[_qp] = (_kB0 + _kB1 * _temperature[_qp] / 1000 + _kB2 * Utility::pow<2>(_temperature[_qp] / 1000) + _kB3 * Utility::pow<3>(_temperature[_qp] / 1000))/(_kC0 + _kC1 * _temperature[_qp] / 1000+ Utility::pow<2>(_temperature[_qp] / 1000));
}