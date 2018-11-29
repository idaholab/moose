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
  InputParameters params = validParams<ThermalSolidPropertiesMaterial>();
  params.addClassDescription("ThermalSolidPropertiesMaterial defining stainless steel 316 thermal "
                             "properties.");

  params.addParam<MooseEnum>(
      "surface", getSurfaceEnum("oxidized"), "The state of the solid surface");
  params.addRangeCheckedParam<Real>("emissivity",
                                    1.0,
                                    "emissivity >= 0.0 & emissivity <= 1.0",
                                    "Optional user-specified constant emissivity");
  return params;
}

ThermalStainlessSteel316Properties::ThermalStainlessSteel316Properties(const InputParameters & parameters)
  : ThermalSolidPropertiesMaterial(parameters),
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

void
ThermalStainlessSteel316Properties::computeIsobaricSpecificHeat()
{
  _cp[_qp] = 0.1816 * _temperature[_qp] + 428.46;
}

void
ThermalStainlessSteel316Properties::computeIsobaricSpecificHeatDerivatives()
{
  _dcp_dT[_qp] = 0.1816;
}

void
ThermalStainlessSteel316Properties::computeThermalConductivity()
{
  _k[_qp] = -7.301e-6 * _temperature[_qp] * _temperature[_qp] + 0.02716 * _temperature[_qp] + 6.308;
}

void
ThermalStainlessSteel316Properties::computeThermalConductivityDerivatives()
{
  _dk_dT[_qp] = -7.301e-6 * 2.0 * _temperature[_qp] + 0.02716;
}

void
ThermalStainlessSteel316Properties::computeDensity()
{
  _rho[_qp] = -4.454e-5 * _temperature[_qp] * _temperature[_qp] - 0.4297 * _temperature[_qp] + 8089.4;
}

void
ThermalStainlessSteel316Properties::computeDensityDerivatives()
{
  _drho_dT[_qp] = -4.454e-5 * 2.0 * _temperature[_qp] - 0.4297;
}

//Real
//ThermalStainlessSteel316Properties::surface_emissivity() const
//{
//  if (_constant_emissivity)
//    return _emissivity;
//
//  if (_surface == surface::oxidized)
//    return 0.7;
//  else if (_surface == surface::polished)
//    return 0.15;
//  else
//    mooseError(name(), ": Unhandled SurfaceEnum in 'emissivity_from_T'!");
//}
