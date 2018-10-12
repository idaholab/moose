//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSolidProperties.h"

const std::string ThermalSolidProperties::_name = "";

template <>
InputParameters
validParams<ThermalSolidProperties>()
{
  InputParameters params = validParams<SolidProperties>();
  params.addClassDescription("Userobject defining solid thermal properties");
  return params;
}

ThermalSolidProperties::ThermalSolidProperties(const InputParameters & parameters)
  : SolidProperties(parameters)
{
}

const std::string &
ThermalSolidProperties::solidName() const
{
  return _name;
}

Real
ThermalSolidProperties::cp_from_T(Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
ThermalSolidProperties::cp_from_T(Real, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
ThermalSolidProperties::k_from_T(Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
ThermalSolidProperties::k_from_T(Real, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
ThermalSolidProperties::rho_from_T(Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void
ThermalSolidProperties::rho_from_T(Real, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real
ThermalSolidProperties::beta_from_T(Real T) const
{
  Real rho, drho_dT;
  rho_from_T(T, rho, drho_dT);
  return - drho_dT / rho;
}

Real
ThermalSolidProperties::emissivity_from_T(Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}
