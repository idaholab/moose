//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSolidProperties.h"

InputParameters
ThermalSolidProperties::validParams()
{
  InputParameters params = SolidProperties::validParams();

  // There does not seem to be a conventional reference temperature, so STP was chosen.
  params.addParam<Real>(
      "T_zero_e",
      273.15,
      "Temperature at which the specific internal energy is assumed to be zero [K].");

  return params;
}

ThermalSolidProperties::ThermalSolidProperties(const InputParameters & parameters)
  : SolidProperties(parameters), _T_zero_e(getParam<Real>("T_zero_e"))
{
}

Real
ThermalSolidProperties::e_from_T(const Real & T) const
{
  return cp_integral(T) - cp_integral(_T_zero_e);
}

void
ThermalSolidProperties::e_from_T(const Real & T, Real & e, Real & de_dT) const
{
  e = e_from_T(T);
  de_dT = cp_from_T(T);
}
