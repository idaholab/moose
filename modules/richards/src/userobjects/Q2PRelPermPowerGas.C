//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  "PowerGas" form of relative permeability
//
#include "Q2PRelPermPowerGas.h"

registerMooseObject("RichardsApp", Q2PRelPermPowerGas);

InputParameters
Q2PRelPermPowerGas::validParams()
{
  InputParameters params = RichardsRelPerm::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "simm",
      "simm >= 0 & simm < 1",
      "Immobile saturation.  Must be between 0 and 1.   Define s = "
      "seff/(1 - simm).  Then relperm = 1 - (n+1)s^n + ns^(n+1)");
  params.addRequiredRangeCheckedParam<Real>("n",
                                            "n >= 2",
                                            "Exponent.  Must be >= 2.   Define s = "
                                            "(eff/(1 - simm).  Then relperm = 1 - "
                                            "(n+1)s^n + ns^(n+1)");
  params.addClassDescription("Power form of relative permeability that might be useful for gases "
                             "as a function of water saturation in Q2P models.  Define s = seff/(1 "
                             "- simm).  Then relperm = 1 - (n+1)s^n + ns^(n+1) if seff<1-simm, "
                             "otherwise relperm=1.  Here seff is the water saturation");
  return params;
}

Q2PRelPermPowerGas::Q2PRelPermPowerGas(const InputParameters & parameters)
  : RichardsRelPerm(parameters), _simm(getParam<Real>("simm")), _n(getParam<Real>("n"))
{
}

Real
Q2PRelPermPowerGas::relperm(Real seff) const
{
  if (seff >= 1.0 - _simm)
    return 0.0;

  if (seff <= 0)
    return 1.0;

  Real s_internal = seff / (1.0 - _simm);
  Real krel = 1 - (_n + 1) * std::pow(s_internal, _n) + _n * std::pow(s_internal, _n + 1);

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
Q2PRelPermPowerGas::drelperm(Real seff) const
{
  if (seff >= 1.0 - _simm)
    return 0.0;

  if (seff <= 0)
    return 0.0;

  Real s_internal = seff / (1.0 - _simm);
  Real krelp =
      -(_n + 1) * _n * std::pow(s_internal, _n - 1) + _n * (_n + 1) * std::pow(s_internal, _n);
  return krelp / (1.0 - _simm);
}

Real
Q2PRelPermPowerGas::d2relperm(Real seff) const
{
  if (seff >= 1.0 - _simm)
    return 0.0;

  if (seff <= 0)
    return 0.0;

  Real s_internal = seff / (1.0 - _simm);
  Real krelpp = -(_n + 1) * _n * (_n - 1) * std::pow(s_internal, _n - 2) +
                _n * (_n + 1) * _n * std::pow(s_internal, _n - 1);
  return krelpp / std::pow(1.0 - _simm, 2);
}
