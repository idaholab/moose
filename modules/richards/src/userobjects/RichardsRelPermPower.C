/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  "Power" form of relative permeability
//
#include "RichardsRelPermPower.h"

template<>
InputParameters validParams<RichardsRelPermPower>()
{
  InputParameters params = validParams<RichardsRelPerm>();
  params.addRequiredParam<Real>("simm", "Immobile saturation.  Must be between 0 and 1.   Define s = (seff - simm)/(1 - simm).  Then relperm = (n+1)s^n - ns^(n+1)");
  params.addRequiredParam<Real>("n", "Exponent.  Must be >= 2.   Define s = (seff - simm)/(1 - simm).  Then relperm = (n+1)s^n - ns^(n+1)");
  params.addClassDescription("Power form of relative permeability.  Define s = (seff - simm)/(1 - simm).  Then relperm = (n+1)s^n - ns^(n+1) if s<simm, otherwise relperm=1");
  return params;
}

RichardsRelPermPower::RichardsRelPermPower(const std::string & name, InputParameters parameters) :
  RichardsRelPerm(name, parameters),
  _simm(getParam<Real>("simm")),
  _n(getParam<Real>("n"))
{
  if (_simm < 0 || _simm > 1)
    mooseError("Immobile saturation set to " << _simm << " in relative permeability function but it must not be less than zero or greater than 1");
  if (_n < 2)
    mooseError("The exponent, n, in the power relative permeability function is set to " << _n << " but it must not be less than 2");
}


Real
RichardsRelPermPower::relperm(Real seff) const
{
  if (seff >= 1.0) {
    return 1.0;
  }

  if (seff <= _simm) {
    return 0.0;
  }

  Real s_internal = (seff - _simm)/(1.0 - _simm);
  Real krel = (_n + 1)*std::pow(s_internal, _n) - _n*std::pow(s_internal, _n + 1);

  // bound, just in case
  if (krel < 0) { krel = 0;}
  if (krel > 1) { krel = 1;}
  return krel;
}


Real
RichardsRelPermPower::drelperm(Real seff) const
{
  if (seff >= 1.0) {
    return 0.0;
  }

  if (seff <= _simm) {
    return 0.0;
  }

  Real s_internal = (seff - _simm)/(1.0 - _simm);
  Real krelp = (_n + 1)*_n*std::pow(s_internal, _n - 1) - _n*(_n + 1)*std::pow(s_internal, _n);
  return krelp/(1.0 - _simm);
}


Real
RichardsRelPermPower::d2relperm(Real seff) const
{
  if (seff >= 1.0) {
    return 0.0;
  }

  if (seff <= _simm) {
    return 0.0;
  }

  Real s_internal = (seff - _simm)/(1.0 - _simm);
  Real krelpp = (_n + 1)*_n*(_n - 1)*std::pow(s_internal, _n - 2) - _n*(_n + 1)*_n*std::pow(s_internal, _n - 1);
  return krelpp/std::pow(1.0 - _simm, 2);
}

