//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  "Power" form of relative permeability
//
#include "RichardsRelPermPower.h"

registerMooseObject("RichardsApp", RichardsRelPermPower);

InputParameters
RichardsRelPermPower::validParams()
{
  InputParameters params = RichardsRelPerm::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "simm",
      "simm >= 0 & simm < 1",
      "Immobile saturation.  Must be between 0 and 1.   Define s = "
      "(seff - simm)/(1 - simm).  Then relperm = (n+1)s^n - "
      "ns^(n+1)");
  params.addRequiredRangeCheckedParam<Real>("n",
                                            "n >= 2",
                                            "Exponent.  Must be >= 2.   Define s = "
                                            "(seff - simm)/(1 - simm).  Then "
                                            "relperm = (n+1)s^n - ns^(n+1)");
  params.addClassDescription("Power form of relative permeability.  Define s = (seff - simm)/(1 - "
                             "simm).  Then relperm = (n+1)s^n - ns^(n+1) if s<simm, otherwise "
                             "relperm=1");
  return params;
}

RichardsRelPermPower::RichardsRelPermPower(const InputParameters & parameters)
  : RichardsRelPerm(parameters), _simm(getParam<Real>("simm")), _n(getParam<Real>("n"))
{
}

Real
RichardsRelPermPower::relperm(Real seff) const
{
  if (seff >= 1.0)
    return 1.0;

  if (seff <= _simm)
    return 0.0;

  Real s_internal = (seff - _simm) / (1.0 - _simm);
  Real krel = (_n + 1) * std::pow(s_internal, _n) - _n * std::pow(s_internal, _n + 1);

  // bound, just in case
  if (krel < 0)
  {
    krel = 0;
  }
  if (krel > 1)
  {
    krel = 1;
  }
  return krel;
}

Real
RichardsRelPermPower::drelperm(Real seff) const
{
  if (seff >= 1.0)
    return 0.0;

  if (seff <= _simm)
    return 0.0;

  Real s_internal = (seff - _simm) / (1.0 - _simm);
  Real krelp =
      (_n + 1) * _n * std::pow(s_internal, _n - 1) - _n * (_n + 1) * std::pow(s_internal, _n);
  return krelp / (1.0 - _simm);
}

Real
RichardsRelPermPower::d2relperm(Real seff) const
{
  if (seff >= 1.0)
    return 0.0;

  if (seff <= _simm)
    return 0.0;

  Real s_internal = (seff - _simm) / (1.0 - _simm);
  Real krelpp = (_n + 1) * _n * (_n - 1) * std::pow(s_internal, _n - 2) -
                _n * (_n + 1) * _n * std::pow(s_internal, _n - 1);
  return krelpp / std::pow(1.0 - _simm, 2);
}
