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

const std::string ThermalGraphiteProperties::_name = std::string("thermal_graphite");

template <>
InputParameters
validParams<ThermalGraphiteProperties>()
{
  InputParameters params = validParams<ThermalSolidProperties>();
  params.addClassDescription("Userobject defining graphite thermal properties.");
  params.addRangeCheckedParam<Real>(
      "density_room_temp", 1600.0, "density_room_temp > 0.0", "Density at room temperature");
  params.addParam<MooseEnum>(
      "surface", getSurfaceEnum("oxidized"), "The state of the solid surface");
  params.addRangeCheckedParam<Real>("emissivity",
                                    1.0,
                                    "emissivity >= 0.0 & emissivity <= 1.0",
                                    "Optional user-specified constant emissivity");
  return params;
}

ThermalGraphiteProperties::ThermalGraphiteProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters),
    _rho_room_temp(getParam<Real>("density_room_temp")),
    _surface(getParam<MooseEnum>("surface").getEnum<surface::SurfaceEnum>()),
    _constant_emissivity(parameters.isParamSetByUser("emissivity") ? true : false),
    _emissivity(getParam<Real>("emissivity")),
    _beta0(2.925e-6)
{
}

const std::string &
ThermalGraphiteProperties::solidName() const
{
  return _name;
}

Real
ThermalGraphiteProperties::molarMass() const
{
  return 12.0107e-3;
}

Real
ThermalGraphiteProperties::cp_from_T(Real T) const
{
  return 4184.0 * (0.54212 - 2.42667e-6 * T - 90.2725 / T - 43449.3 * std::pow(T, -2.0) +
                   1.59309e7 * std::pow(T, -3.0) - 1.43688e9 * std::pow(T, -4.0));
}

void
ThermalGraphiteProperties::cp_from_T(Real T, Real & cp, Real & dcp_dT) const
{
  cp = cp_from_T(T);
  dcp_dT = 4184.0 * (-2.42667e-6 + 90.2725 * std::pow(T, -2.0) + 86898.6 * std::pow(T, -3.0) -
                     4.77927e7 * std::pow(T, -4.0) + 5.74752e9 * std::pow(T, -5.0));
}

Real
ThermalGraphiteProperties::k_from_T(Real T) const
{
  return 2879.68819 * std::pow(T, -0.52813);
}

void
ThermalGraphiteProperties::k_from_T(Real T, Real & k, Real & dk_dT) const
{
  k = k_from_T(T);
  dk_dT = 2879.68819 * -0.52813 * std::pow(T, -1.52813);
}

Real
ThermalGraphiteProperties::rho_from_T(Real T) const
{
  return _rho_room_temp * (1.0 - beta_from_T(T) * (T - 293.15));
}

void
ThermalGraphiteProperties::rho_from_T(Real T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);

  if (T > 373.15)
  {
    Real d_betaT = _beta0 + (2.1e-9 * 4.0 * std::pow(T, 3.0) - 1.23726e-5 * 3.0 * std::pow(T, 2.0) +
                             3.05359e-2 * 2.0 * T - 9.73349) *
                                1e-7;
    Real d_beta = (2.1e-9 * 3.0 * std::pow(T, 2.0) - 1.23726e-5 * 2.0 * T + 3.05359e-2) * 1e-7;
    drho_dT = -_rho_room_temp * (d_betaT - 293.15 * d_beta);
  }
  else
    drho_dT = -_beta0 * _rho_room_temp;
}

Real
ThermalGraphiteProperties::beta_from_T(Real T) const
{
  Real term = 0.0;

  if (T > 373.15)
    term = (2.1e-9 * std::pow(T, 3.0) - 1.23726e-5 * std::pow(T, 2.0) + 3.05359e-2 * T - 9.73349) *
           1e-7;

  return _beta0 + term;
}

Real
ThermalGraphiteProperties::emissivity_from_T(Real T) const
{
  if (_constant_emissivity)
    return _emissivity;

  if (_surface == surface::oxidized)
    return 0.732 + 5.21e-5 * T;
  else if (_surface == surface::polished)
    return 0.448 + 13.8e-5 * T;
  else
    mooseError(name(), ": Unhandled SurfaceEnum in 'emissivity_from_T'!");
}
