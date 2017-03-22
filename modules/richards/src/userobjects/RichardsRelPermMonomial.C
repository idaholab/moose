/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  "Monomial" form of relative permeability
//
#include "RichardsRelPermMonomial.h"

template <>
InputParameters
validParams<RichardsRelPermMonomial>()
{
  InputParameters params = validParams<RichardsRelPerm>();
  params.addRequiredRangeCheckedParam<Real>(
      "simm",
      "simm >= 0 & simm < 1",
      "Immobile saturation.  Must be between 0 and 1.   Define s = "
      "(seff - simm)/(1 - simm).  Then relperm = s^n");
  params.addRequiredRangeCheckedParam<Real>(
      "n",
      "n >= 0",
      "Exponent.  Must be >= 0.   Define s = (seff - simm)/(1 - simm).  Then relperm = s^n");
  params.addParam<Real>(
      "zero_to_the_zero", 0.0, "If n=0, this is the value of relative permeability for s<=simm");
  params.addClassDescription("Monomial form of relative permeability.  Define s = (seff - simm)/(1 "
                             "- simm).  Then relperm = s^n if s<simm, otherwise relperm=1");
  return params;
}

RichardsRelPermMonomial::RichardsRelPermMonomial(const InputParameters & parameters)
  : RichardsRelPerm(parameters),
    _simm(getParam<Real>("simm")),
    _n(getParam<Real>("n")),
    _zero_to_the_zero(getParam<Real>("zero_to_the_zero"))
{
}

Real
RichardsRelPermMonomial::relperm(Real seff) const
{
  if (seff >= 1.0)
    return 1.0;

  if (_n == 0 && seff <= _simm)
    return _zero_to_the_zero;

  if (seff <= _simm)
    return 0.0;

  Real s_internal = (seff - _simm) / (1.0 - _simm);
  Real krel = std::pow(s_internal, _n);

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
RichardsRelPermMonomial::drelperm(Real seff) const
{
  if (seff >= 1.0)
    return 0.0;

  if (seff <= _simm)
    return 0.0;

  if (_n == 0)
    return 0.0;

  Real s_internal = (seff - _simm) / (1.0 - _simm);
  Real krelp = _n * std::pow(s_internal, _n - 1);
  return krelp / (1.0 - _simm);
}

Real
RichardsRelPermMonomial::d2relperm(Real seff) const
{
  if (seff >= 1.0)
    return 0.0;

  if (seff <= _simm)
    return 0.0;

  if (_n == 0)
    return 0.0;

  Real s_internal = (seff - _simm) / (1.0 - _simm);
  Real krelpp = _n * (_n - 1) * std::pow(s_internal, _n - 2);
  return krelpp / std::pow(1.0 - _simm, 2);
}
