//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalGraphiteProperties.h"
#include "libmesh/utility.h"

registerMooseObject("SolidPropertiesApp", ThermalGraphiteProperties);

InputParameters
ThermalGraphiteProperties::validParams()
{
  InputParameters params = ThermalSolidProperties::validParams();

  MooseEnum graphite_grade("H_451");
  params.addRequiredParam<MooseEnum>("grade", graphite_grade, "Graphite grade");
  params.addRangeCheckedParam<Real>("density", 1850.0, "density > 0.0", "(Constant) density");
  params.addClassDescription("Graphite thermal properties.");
  return params;
}

ThermalGraphiteProperties::ThermalGraphiteProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters),
    _grade(getParam<MooseEnum>("grade").getEnum<GraphiteGrade>()),
    _rho_const(getParam<Real>("density"))
{
}

Real
ThermalGraphiteProperties::cp_from_T(const Real & T) const
{
  switch (_grade)
  {
    case GraphiteGrade::H_451:
      return 4184.0 * (0.54212 - 2.42667e-6 * T - 90.2725 / T - 43449.3 / Utility::pow<2>(T) +
                       1.59309e7 / Utility::pow<3>(T) - 1.43688e9 / Utility::pow<4>(T));
    default:
      mooseError("Unhandled GraphiteGrade enum!");
  }
}

void
ThermalGraphiteProperties::cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const
{
  cp = cp_from_T(T);

  switch (_grade)
  {
    case GraphiteGrade::H_451:
    {
      dcp_dT = 4184.0 * (-2.42667e-6 + 90.2725 / Utility::pow<2>(T) + 86898.6 / Utility::pow<3>(T) -
                         4.77927e7 / Utility::pow<4>(T) + 5.74752e9 / Utility::pow<5>(T));
      break;
    }
    default:
      mooseError("Unhandled GraphiteGrade enum!");
  }
}

Real
ThermalGraphiteProperties::k_from_T(const Real & T) const
{
  switch (_grade)
  {
    case GraphiteGrade::H_451:
      return 3.28248e-5 * Utility::pow<2>(T) - 1.24890e-1 * T + 1.692145e2;
    default:
      mooseError("Unhandled GraphiteGrade enum!");
  }
}

void
ThermalGraphiteProperties::k_from_T(const Real & T, Real & k, Real & dk_dT) const
{
  k = k_from_T(T);

  switch (_grade)
  {
    case GraphiteGrade::H_451:
    {
      dk_dT = 6.56496e-5 * T - 1.24890e-1;
      break;
    }
    default:
      mooseError("Unhandled GraphiteGrade enum!");
  }
}

Real
ThermalGraphiteProperties::rho_from_T(const Real & /* T */) const
{
  return _rho_const;
}

void
ThermalGraphiteProperties::rho_from_T(const Real & T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);
  drho_dT = 0.0;
}
