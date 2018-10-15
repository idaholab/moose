//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalStainlessSteel316Properties.h"

registerMooseObject("SolidPropertiesApp", ThermalStainlessSteel316Properties);

const std::string ThermalStainlessSteel316Properties::_name =
    std::string("thermal_stainless_steel_316");

template <>
InputParameters
validParams<ThermalStainlessSteel316Properties>()
{
  InputParameters params = validParams<ThermalSolidProperties>();
  params.addClassDescription("Userobject defining stainless steel 316 thermal "
                             "properties.");

  params.addParam<MooseEnum>(
      "surface", getSurfaceEnum("oxidized"), "The state of the solid surface");
  params.addRangeCheckedParam<Real>("emissivity",
                                    1.0,
                                    "emissivity >= 0.0 & emissivity <= 1.0",
                                    "Optional user-specified constant emissivity");
  return params;
}

ThermalStainlessSteel316Properties::ThermalStainlessSteel316Properties(
    const InputParameters & parameters)
  : ThermalSolidProperties(parameters),
    _surface(getParam<MooseEnum>("surface").getEnum<surface::SurfaceEnum>()),
    _constant_emissivity(parameters.isParamSetByUser("emissivity") ? true : false),
    _emissivity(getParam<Real>("emissivity"))
{
}

const std::string &
ThermalStainlessSteel316Properties::solidName() const
{
  return _name;
}

Real
ThermalStainlessSteel316Properties::molarMass() const
{
  return 55.34175166872447e-3;
}

Real
ThermalStainlessSteel316Properties::cp_from_T(Real T) const
{
  return 0.1816 * T + 428.46;
}

void
ThermalStainlessSteel316Properties::cp_from_T(Real T, Real & cp, Real & dcp_dT) const
{
  cp = cp_from_T(T);
  dcp_dT = 0.1816;
}

Real
ThermalStainlessSteel316Properties::k_from_T(Real T) const
{
  return -7.301e-6 * T * T + 0.02716 * T + 6.308;
}

void
ThermalStainlessSteel316Properties::k_from_T(Real T, Real & k, Real & dk_dT) const
{
  k = k_from_T(T);
  dk_dT = -7.301e-6 * 2.0 * T + 0.02716;
}

Real
ThermalStainlessSteel316Properties::rho_from_T(Real T) const
{
  return -4.454e-5 * T * T - 0.4297 * T + 8089.4;
}

void
ThermalStainlessSteel316Properties::rho_from_T(Real T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);
  drho_dT = -4.454e-5 * 2.0 * T - 0.4297;
}

Real ThermalStainlessSteel316Properties::emissivity_from_T(Real /* T */) const
{
  if (_constant_emissivity)
    return _emissivity;

  if (_surface == surface::oxidized)
    return 0.7;
  else if (_surface == surface::polished)
    return 0.15;
  else
    mooseError(name(), ": Unhandled SurfaceEnum in 'emissivity_from_T'!");
}
