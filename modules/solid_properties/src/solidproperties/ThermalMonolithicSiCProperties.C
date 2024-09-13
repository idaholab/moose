//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalMonolithicSiCProperties.h"
#include "libmesh/utility.h"

registerMooseObject("SolidPropertiesApp", ThermalMonolithicSiCProperties);

InputParameters
ThermalMonolithicSiCProperties::validParams()
{
  InputParameters params = ThermalSolidProperties::validParams();

  MooseEnum ThermalConductivityModel("SNEAD STONE", "SNEAD");
  params.addParam<MooseEnum>("thermal_conductivity_model",
                             ThermalConductivityModel,
                             "Thermal conductivity model to be used");
  params.addRangeCheckedParam<Real>("density", 3216.0, "density > 0.0", "(Constant) density");
  params.addClassDescription("Monolithic silicon carbide thermal properties.");
  return params;
}

ThermalMonolithicSiCProperties::ThermalMonolithicSiCProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters),
    _k_model(getParam<MooseEnum>("thermal_conductivity_model").getEnum<ThermalConductivityModel>()),
    _rho_const(getParam<Real>("density")),
    _c1(925.65),
    _c2(0.3772),
    _c3(7.9259e-5),
    _c4(3.1946e7)
{
}

Real
ThermalMonolithicSiCProperties::cp_from_T(const Real & T) const
{
  return _c1 + _c2 * T - _c3 * Utility::pow<2>(T) - _c4 / Utility::pow<2>(T);
}

void
ThermalMonolithicSiCProperties::cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const
{
  cp = cp_from_T(T);
  dcp_dT = _c2 - 2 * _c3 * T + 2 * _c4 / Utility::pow<3>(T);
}

Real
ThermalMonolithicSiCProperties::cp_integral(const Real & T) const
{
  return _c1 * T + 0.5 * _c2 * Utility::pow<2>(T) - _c3 / 3.0 * Utility::pow<3>(T) + _c4 / T;
}

Real
ThermalMonolithicSiCProperties::k_from_T(const Real & T) const
{
  switch (_k_model)
  {
    case ThermalConductivityModel::SNEAD:
      return 1.0 / (-0.0003 + 1.05e-5 * T);
    case ThermalConductivityModel::STONE:
      return -3.70e-8 * Utility::pow<3>(T) + 1.54e-4 * Utility::pow<2>(T) - 0.214 * T + 153.1;
    default:
      mooseError("Unhandled MooseEnum in ThermalMonolithicSiCProperties!");
  }
}

void
ThermalMonolithicSiCProperties::k_from_T(const Real & T, Real & k, Real & dk_dT) const
{
  k = k_from_T(T);

  switch (_k_model)
  {
    case ThermalConductivityModel::SNEAD:
    {
      dk_dT = -1.0 / Utility::pow<2>(-0.0003 + 1.05e-5 * T) * 1.05e-5;
      break;
    }
    case ThermalConductivityModel::STONE:
    {
      dk_dT = -1.11e-7 * Utility::pow<2>(T) + 3.08E-4 * T - 0.214;
      break;
    }
    default:
      mooseError("Unhandled MooseEnum in ThermalMonolithicSiCProperties!");
  }
}

Real
ThermalMonolithicSiCProperties::rho_from_T(const Real & /* T */) const
{
  return _rho_const;
}

void
ThermalMonolithicSiCProperties::rho_from_T(const Real & T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);
  drho_dT = 0.0;
}
