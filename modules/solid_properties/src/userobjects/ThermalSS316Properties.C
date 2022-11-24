//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSS316Properties.h"
#include "libmesh/utility.h"

registerMooseObject("SolidPropertiesApp", ThermalSS316Properties);

InputParameters
ThermalSS316Properties::validParams()
{
  InputParameters params = ThermalSolidProperties::validParams();
  params.addClassDescription("Stainless steel 316 thermal properties.");
  return params;
}

ThermalSS316Properties::ThermalSS316Properties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters)
{
}

Real
ThermalSS316Properties::cp_from_T(const Real & T) const
{
  return 0.1816 * T + 428.46;
}

void
ThermalSS316Properties::cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const
{
  cp = cp_from_T(T);
  dcp_dT = 0.1816;
}

Real
ThermalSS316Properties::k_from_T(const Real & T) const
{
  return -7.301e-6 * Utility::pow<2>(T) + 0.02716 * T + 6.308;
}

void
ThermalSS316Properties::k_from_T(const Real & T, Real & k, Real & dk_dT) const
{
  k = k_from_T(T);
  dk_dT = -1.4602e-5 * T + 0.02716;
}

Real
ThermalSS316Properties::rho_from_T(const Real & T) const
{
  return -4.454e-5 * Utility::pow<2>(T) - 0.4297 * T + 8089.4;
}

void
ThermalSS316Properties::rho_from_T(const Real & T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);
  drho_dT = -8.908e-5 * T - 0.4297;
}
