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
    _rho_const(getParam<Real>("density")),
    _c1(4184.0),
    _c2(0.54212),
    _c3(2.42667e-6),
    _c4(90.2725),
    _c5(43449.3),
    _c6(1.59309e7),
    _c7(1.43688e9)
{
}

Real
ThermalGraphiteProperties::cp_from_T(const Real & T) const
{
  switch (_grade)
  {
    case GraphiteGrade::H_451:
      return _c1 * (_c2 - _c3 * T - _c4 / T - _c5 / Utility::pow<2>(T) + _c6 / Utility::pow<3>(T) -
                    _c7 / Utility::pow<4>(T));
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
      dcp_dT = _c1 * (-_c3 + _c4 / Utility::pow<2>(T) + 2.0 * _c5 / Utility::pow<3>(T) -
                      3.0 * _c6 / Utility::pow<4>(T) + 4.0 * _c7 / Utility::pow<5>(T));
      break;
    }
    default:
      mooseError("Unhandled GraphiteGrade enum!");
  }
}

Real
ThermalGraphiteProperties::cp_integral(const Real & T) const
{
  switch (_grade)
  {
    case GraphiteGrade::H_451:
    {
      return _c1 * (_c2 * T - 0.5 * _c3 * Utility::pow<2>(T) - _c4 * std::log(T) + _c5 / T -
                    0.5 * _c6 / Utility::pow<2>(T) + _c7 / (3.0 * Utility::pow<3>(T)));
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
