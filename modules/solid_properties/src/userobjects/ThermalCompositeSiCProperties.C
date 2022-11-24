//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalCompositeSiCProperties.h"
#include "libmesh/utility.h"

registerMooseObject("SolidPropertiesApp", ThermalCompositeSiCProperties);

InputParameters
ThermalCompositeSiCProperties::validParams()
{
  InputParameters params = ThermalSolidProperties::validParams();

  params.addRangeCheckedParam<Real>("density", 3216.0, "density > 0.0", "(Constant) density");
  params.addClassDescription("Composite silicon carbide thermal properties.");
  return params;
}

ThermalCompositeSiCProperties::ThermalCompositeSiCProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters), _rho_const(getParam<Real>("density"))
{
}

Real
ThermalCompositeSiCProperties::cp_from_T(const Real & T) const
{
  return 925.65 + 0.3772 * T - 7.9259e-5 * Utility::pow<2>(T) - 3.1946e7 / Utility::pow<2>(T);
}

void
ThermalCompositeSiCProperties::cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const
{
  cp = cp_from_T(T);
  dcp_dT = 0.3772 - 1.58518e-4 * T + 6.3892e7 / Utility::pow<3>(T);
}

Real
ThermalCompositeSiCProperties::k_from_T(const Real & T) const
{
  return -1.71e-11 * Utility::pow<4>(T) + 7.35e-8 * Utility::pow<3>(T) -
         1.10e-4 * Utility::pow<2>(T) + 0.061 * T + 7.97;
}

void
ThermalCompositeSiCProperties::k_from_T(const Real & T, Real & k, Real & dk_dT) const
{
  k = k_from_T(T);
  dk_dT = -6.84e-11 * Utility::pow<3>(T) + 2.205e-7 * Utility::pow<2>(T) - 2.2e-4 * T + 0.061;
}

Real
ThermalCompositeSiCProperties::rho_from_T(const Real & /* T */) const
{
  return _rho_const;
}

void
ThermalCompositeSiCProperties::rho_from_T(const Real & T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);
  drho_dT = 0.0;
}
