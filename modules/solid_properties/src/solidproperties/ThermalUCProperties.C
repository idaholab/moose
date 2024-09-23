//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalUCProperties.h"
#include "libmesh/utility.h"

registerMooseObject("SolidPropertiesApp", ThermalUCProperties);

InputParameters
ThermalUCProperties::validParams()

{
  InputParameters params = ThermalSolidProperties::validParams();

  params.addRangeCheckedParam<Real>("density", 13824.7, "density > 0.0", "(Constant) density");
  params.addClassDescription("Uranium Carbide (UC) thermal properties (SI units).");
  return params;
}

ThermalUCProperties::ThermalUCProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters),
    _rho_const(getParam<Real>("density")),
    _c1(239.7),
    _c2(5.068e-3),
    _c3(1.7604e-5),
    _c4(3488100)
{
}

Real
ThermalUCProperties::cp_from_T(const Real & T) const
{
  if ((T < 298) || (T > 2838))
    flagInvalidSolution(
        "UC specific heat evaluated outside of UC cp temperature range [298, 2838] K");
  return _c1 - _c2 * T + _c3 * Utility::pow<2>(T) - _c4 / Utility::pow<2>(T);
}

void
ThermalUCProperties::cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const
{

  if ((T < 298) || (T > 2838))
    flagInvalidSolution(
        "UC specific heat evaluated outside of UC cp temperature range [298, 2838] K");

  cp = cp_from_T(T);
  dcp_dT = -_c2 + 2 * _c3 * T + 2 * _c4 / Utility::pow<3>(T);
}

Real
ThermalUCProperties::cp_integral(const Real & T) const
{
  return _c1 * T - 0.5 * _c2 * Utility::pow<2>(T) + _c3 / 3.0 * Utility::pow<3>(T) + _c4 / T;
}

Real
ThermalUCProperties::k_from_T(const Real & T) const
{
  if ((323 < T) && (T < 924))
  {
    return 21.7 - 3.04e-3 * T + 3.61e-6 * Utility::pow<2>(T);
  }
  else if ((924 < T) && (T < 2573))
  {
    return 20.2 + 1.48e-3 * T;
  }
  else
  {
    return 21.0;
  }
}

void
ThermalUCProperties::k_from_T(const Real & T, Real & k, Real & dk_dT) const
{
  if ((323 < T) && (T < 924))
  {
    k = k_from_T(T);
    dk_dT = -3.04e-3 + 7.22e-6 * T;
  }
  else if ((924 < T) && (T < 2573))
  {
    k = k_from_T(T);
    dk_dT = 1.48e-3;
  }
  else
  {
    k = k_from_T(T);
    dk_dT = 0.0;
  }
}

Real
ThermalUCProperties::rho_from_T(const Real & /* T */) const
{
  return _rho_const;
}

void
ThermalUCProperties::rho_from_T(const Real & T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);
  drho_dT = 0.0;
}
