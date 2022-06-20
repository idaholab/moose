//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSiliconCarbideProperties.h"
#include "libmesh/utility.h"

registerMooseObject("SolidPropertiesApp", ThermalSiliconCarbideProperties);

InputParameters
ThermalSiliconCarbideProperties::validParams()
{
  InputParameters params = ThermalSolidProperties::validParams();

  MooseEnum ThermalConductivityModel("SNEAD PARFUME", "SNEAD");
  params.addParam<MooseEnum>("thermal_conductivity_model",
                             ThermalConductivityModel,
                             "Thermal conductivity model to be used");
  params.addRangeCheckedParam<Real>("density", 3216.0, "density > 0.0", "(Constant) density");
  params.addClassDescription("Silicon carbide thermal properties.");
  return params;
}

ThermalSiliconCarbideProperties::ThermalSiliconCarbideProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters),
    _k_model(getParam<MooseEnum>("thermal_conductivity_model")
                 .getEnum<ThermalConductivityModel>()),
    _rho_const(getParam<Real>("density"))
{
}

Real
ThermalSiliconCarbideProperties::cp_from_T(const Real & T) const
{
  return 925.65 + 0.3772 * T - 7.9259e-5 * Utility::pow<2>(T) - 3.1946e7 / Utility::pow<2>(T);
}

void
ThermalSiliconCarbideProperties::cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const
{
  cp = cp_from_T(T);
  dcp_dT = 0.3772 - 1.58518e-4 * T + 6.3892e7 / Utility::pow<3>(T);
}

void
ThermalSiliconCarbideProperties::cp_from_T(const DualReal & T, DualReal & cp, DualReal & dcp_dT) const
{
  cp = ThermalSolidProperties::cp_from_T(T);
  dcp_dT = 0.3772 - 1.58518e-4 * T + 6.3892e7 / Utility::pow<3>(T);
}

Real
ThermalSiliconCarbideProperties::k_from_T(const Real & T) const
{
  switch (_k_model)
  {
    case ThermalConductivityModel::SNEAD:
      return 1.0 / (-0.0003 + 1.05e-5 * T);
    case ThermalConductivityModel::PARFUME:
      return 17885.0 / T + 2.0;
    default:
      mooseError("Unhandled MooseEnum in ThermalSiliconCarbideProperties!");
  }
}

void
ThermalSiliconCarbideProperties::k_from_T(const Real & T, Real & k, Real & dk_dT) const
{
  k = k_from_T(T);

  if (_k_model == ThermalConductivityModel::SNEAD)
    dk_dT = -1.0 / Utility::pow<2>(-0.0003 + 1.05e-5 * T) * 1.05e-5;
  else if (_k_model == ThermalConductivityModel::PARFUME)
    dk_dT = -17885.0 / Utility::pow<2>(T);
  else
    mooseError("Unhandled MooseEnum in ThermalSiliconCarbideProperties!");
}

void
ThermalSiliconCarbideProperties::k_from_T(const DualReal & T, DualReal & k, DualReal & dk_dT) const
{
  k = ThermalSolidProperties::k_from_T(T);

  if (_k_model == ThermalConductivityModel::SNEAD)
    dk_dT = -1.0 / Utility::pow<2>(-0.0003 + 1.05e-5 * T) * 1.05e-5;
  else if (_k_model == ThermalConductivityModel::PARFUME)
    dk_dT = -17885.0 / Utility::pow<2>(T);
  else
    mooseError("Unhandled MooseEnum in ThermalSiliconCarbideProperties!");
}

Real
ThermalSiliconCarbideProperties::rho_from_T(const Real & /* T */) const
{
  return _rho_const;
}

void
ThermalSiliconCarbideProperties::rho_from_T(const Real & T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);
  drho_dT = 0.0;
}

void
ThermalSiliconCarbideProperties::rho_from_T(const DualReal & T, DualReal & rho, DualReal & drho_dT) const
{
  rho = ThermalSolidProperties::rho_from_T(T);
  drho_dT = 0.0;
}
