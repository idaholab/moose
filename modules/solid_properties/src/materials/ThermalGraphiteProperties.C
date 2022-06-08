//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalGraphiteProperties.h"

registerMooseObject("SolidPropertiesApp", ThermalGraphiteProperties);

InputParameters
ThermalGraphiteProperties::validParams()
{
  InputParameters params = ThermalSolidPropertiesMaterial::validParams();
  params.addClassDescription("Userobject defining graphite thermal properties.");
  params.addRangeCheckedParam<Real>(
      "density_room_temp", 1600.0, "density_room_temp > 0.0", "Density at room temperature");
  return params;
}

ThermalGraphiteProperties::ThermalGraphiteProperties(const InputParameters & parameters)
  : ThermalSolidPropertiesMaterial(parameters),
    _rho_room_temp(getParam<Real>("density_room_temp")),
    _beta0(2.925e-6)
{
}

Real
ThermalGraphiteProperties::molarMass() const
{
  return 12.0107e-3;
}

void
ThermalGraphiteProperties::computeIsobaricSpecificHeat()
{
  _cp[_qp] = 4184.0 * (0.54212 - 2.42667e-6 * _temperature[_qp] - 90.2725 / _temperature[_qp] -
                       43449.3 * std::pow(_temperature[_qp], -2.0) +
                       1.59309e7 * std::pow(_temperature[_qp], -3.0) -
                       1.43688e9 * std::pow(_temperature[_qp], -4.0));
}

void
ThermalGraphiteProperties::computeIsobaricSpecificHeatDerivatives()
{
  _dcp_dT[_qp] = 4184.0 * (-2.42667e-6 + 90.2725 * std::pow(_temperature[_qp], -2.0) +
                           86898.6 * std::pow(_temperature[_qp], -3.0) -
                           4.77927e7 * std::pow(_temperature[_qp], -4.0) +
                           5.74752e9 * std::pow(_temperature[_qp], -5.0));
}

void
ThermalGraphiteProperties::computeThermalConductivity()
{
  _k[_qp] = 2879.68819 * std::pow(_temperature[_qp], -0.52813);
}

void
ThermalGraphiteProperties::computeThermalConductivityDerivatives()
{
  _dk_dT[_qp] = 2879.68819 * -0.52813 * std::pow(_temperature[_qp], -1.52813);
}

void
ThermalGraphiteProperties::computeDensity()
{
  _rho[_qp] = _rho_room_temp * (1.0 - beta() * (_temperature[_qp] - 293.15));
}

void
ThermalGraphiteProperties::computeDensityDerivatives()
{
  if (_temperature[_qp] > 373.15)
  {
    Real d_betaT = _beta0 + (2.1e-9 * 4.0 * std::pow(_temperature[_qp], 3.0) -
                             1.23726e-5 * 3.0 * std::pow(_temperature[_qp], 2.0) +
                             3.05359e-2 * 2.0 * _temperature[_qp] - 9.73349) *
                                1e-7;
    Real d_beta = (2.1e-9 * 3.0 * std::pow(_temperature[_qp], 2.0) -
                   1.23726e-5 * 2.0 * _temperature[_qp] + 3.05359e-2) *
                  1e-7;
    _drho_dT[_qp] = -_rho_room_temp * (d_betaT - 293.15 * d_beta);
  }
  else
    _drho_dT[_qp] = -_beta0 * _rho_room_temp;
}

Real
ThermalGraphiteProperties::beta() const
{
  Real term = 0.0;

  if (_temperature[_qp] > 373.15)
    term =
        (2.1e-9 * std::pow(_temperature[_qp], 3.0) - 1.23726e-5 * std::pow(_temperature[_qp], 2.0) +
         3.05359e-2 * _temperature[_qp] - 9.73349) *
        1e-7;

  return _beta0 + term;
}
