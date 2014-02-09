/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  "Monomial" form of relative permeability
//
#include "RichardsRelPermMonomial.h"

template<>
InputParameters validParams<RichardsRelPermMonomial>()
{
  InputParameters params = validParams<RichardsRelPerm>();
  params.addRequiredParam<Real>("simm", "Immobile saturation.  Must be between 0 and 1.   Define s = (seff - simm)/(1 - simm).  Then relperm = s^n");
  params.addRequiredParam<Real>("n", "Exponent.  Must be >= 0.   Define s = (seff - simm)/(1 - simm).  Then relperm = s^n");
  params.addClassDescription("Monomial form of relative permeability.  Define s = (seff - simm)/(1 - simm).  Then relperm = s^n if s<simm, otherwise relperm=1");
  return params;
}

RichardsRelPermMonomial::RichardsRelPermMonomial(const std::string & name, InputParameters parameters) :
  RichardsRelPerm(name, parameters),
  _simm(getParam<Real>("simm")),
  _n(getParam<Real>("n"))
{
  if (_simm < 0 || _simm > 1)
    mooseError("Immobile saturation set to " << _simm << " in relative permeability function but it must not be less than zero or greater than 1");
  if (_n < 0)
    mooseError("The exponent, n, in the monomial relative permeability function is set to " << _n << " but it must not be less than 0");
}


Real
RichardsRelPermMonomial::relperm(Real seff) const
{
  if (seff >= 1.0) {
    return 1.0;
  }

  if (seff <= _simm) {
    return 0.0;
  }

  Real s_internal = (seff - _simm)/(1.0 - _simm);
  Real krel = std::pow(s_internal, _n);

  // bound, just in case
  if (krel < 0) { krel = 0;}
  if (krel > 1) { krel = 1;}
  return krel;
}


Real
RichardsRelPermMonomial::drelperm(Real seff) const
{
  if (seff >= 1.0) {
    return 0.0;
  }

  if (seff <= _simm) {
    return 0.0;
  }

  Real s_internal = (seff - _simm)/(1.0 - _simm);
  Real krelp = _n*std::pow(s_internal, _n - 1);
  return krelp/(1.0 - _simm);
}


Real
RichardsRelPermMonomial::d2relperm(Real seff) const
{
  if (seff >= 1.0) {
    return 0.0;
  }

  if (seff <= _simm) {
    return 0.0;
  }

  Real s_internal = (seff - _simm)/(1.0 - _simm);
  Real krelpp = _n*(_n - 1)*std::pow(s_internal, _n - 2);
  return krelpp/std::pow(1.0 - _simm, 2);
}

