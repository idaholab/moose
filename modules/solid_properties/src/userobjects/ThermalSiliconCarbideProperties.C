//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSiliconCarbideProperties.h"

registerMooseObject("SolidPropertiesApp", ThermalSiliconCarbideProperties);

const std::string ThermalSiliconCarbideProperties::_name = std::string("thermal_silicon_carbide");

template <>
InputParameters
validParams<ThermalSiliconCarbideProperties>()
{
  InputParameters params = validParams<ThermalSolidProperties>();
  params.addClassDescription("Userobject defining silicon carbide thermal properties.");

  MooseEnum ThermalSiliconCarbidePropertiesKModel("snead parfume", "snead");
  params.addParam<MooseEnum>("thermal_conductivity_model",
                             ThermalSiliconCarbidePropertiesKModel,
                             "Thermal conductivity model to be used");
  params.addRangeCheckedParam<Real>("density", 3216.0, "density > 0.0", "(Constant) density");
  return params;
}

ThermalSiliconCarbideProperties::ThermalSiliconCarbideProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters),
    _k_model(getParam<MooseEnum>("thermal_conductivity_model")
                 .getEnum<ThermalSiliconCarbidePropertiesKModel>()),
    _rho(getParam<Real>("density"))
{
}

const std::string &
ThermalSiliconCarbideProperties::solidName() const
{
  return _name;
}

Real
ThermalSiliconCarbideProperties::molarMass() const
{
  return 40.0962e-3;
}

Real
ThermalSiliconCarbideProperties::cp_from_T(Real T) const
{
  return 925.65 + 0.3772 * T - 7.9259e-5 * std::pow(T, 2.0) - 3.1946e7 * std::pow(T, -2.0);
}

void
ThermalSiliconCarbideProperties::cp_from_T(Real T, Real & cp, Real & dcp_dT) const
{
  cp = cp_from_T(T);
  dcp_dT = 0.3772 - 7.9259e-5 * 2.0 * T - 3.1946e7 * -2.0 * std::pow(T, -3.0);
}

Real
ThermalSiliconCarbideProperties::k_from_T(Real T) const
{
  if (_k_model == snead)
    return 1.0 / (-0.0003 + 1.05e-5 * T);
  else if (_k_model == parfume)
    return 17885.0 / T + 2.0;
  else
    mooseError(name(), ": Unhandled MooseEnum in ThermalSiliconCarbideProperties!");
}

void
ThermalSiliconCarbideProperties::k_from_T(Real T, Real & k, Real & dk_dT) const
{
  k = k_from_T(T);

  if (_k_model == snead)
    dk_dT = -1.0 / std::pow(-0.0003 + 1.05e-5 * T, 2.0) * 1.05e-5;
  else if (_k_model == parfume)
    dk_dT = -17885.0 / std::pow(T, 2.0);
  else
    mooseError(name(), ": Unhandled MooseEnum in ThermalSiliconCarbideProperties!");
}

Real ThermalSiliconCarbideProperties::rho_from_T(Real /* T */) const { return _rho; }

void
ThermalSiliconCarbideProperties::rho_from_T(Real T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);
  drho_dT = 0.0;
}
