/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  "Broadbridge-White" form of relative permeability (P Broadbridge and I White ``Constant rate rainfall infiltration: A versatile nonlinear model 1. Analytic Solution'', Water Resources Research 24 (1988) 145-154)
//
#include "RichardsRelPermBW.h"

template<>
InputParameters validParams<RichardsRelPermBW>()
{
  InputParameters params = validParams<RichardsRelPerm>();
  params.addRequiredParam<Real>("Sn", "Low saturation.  This must be < Ss, and non-negative.  This is BW's initial effective saturation, below which effective saturation never goes in their simulations/models.  If Kn=0 then Sn is the immobile saturation.");
  params.addParam<Real>("Ss", 1.0, "High saturation.  This must be > Sn and <= 1.  Effective saturation where porepressure = 0.  Effective saturation never exceeds this value in BW's simulations/models.");
  params.addParam<Real>("Kn", 0.0, "Relative permeability at Seff = Sn.  Must be < Ks");
  params.addParam<Real>("Ks", 1.0, "Relative permeability at Seff = Ss.  Must be > Kn");
  params.addRequiredParam<Real>("C", "BW's C parameter.  Must be > 1.   Define s = (seff - Sn)/(Ss - Sn).  Then relperm = Kn + s^2(c-1)(Kn-Ks)/(c-s) if 0<s<1, otherwise relperm = Kn if s<=0, otherwise relperm = Ks if s>=1.");
  params.addClassDescription("Broadbridge-White form of relative permeability.  Define s = (seff - Sn)/(Ss - Sn).  Then relperm = Kn + s^2(c-1)(Kn-Ks)/(c-s) if 0<s<1, otherwise relperm = Kn if s<=0, otherwise relperm = Ks if s>=1.");
  return params;
}

RichardsRelPermBW::RichardsRelPermBW(const std::string & name, InputParameters parameters) :
  RichardsRelPerm(name, parameters),
  _sn(getParam<Real>("Sn")),
  _ss(getParam<Real>("Ss")),
  _kn(getParam<Real>("Kn")),
  _ks(getParam<Real>("Ks")),
  _c(getParam<Real>("C"))
{
  if (_sn < 0)
    mooseError("Sn in BW relative permeability set to " << _sn << " but it must be non-negative");
  if (_ss > 1)
    mooseError("Ss in BW relative permeability set to " << _ss << " but it must not be greater than 1");
  if (_ss <= _sn)
    mooseError("In BW relative permeability Sn set to " << _sn << " and Ss set to " << _ss << " but these must obey Ss > Sn");
  if (_kn < 0)
    mooseError("Kn in BW relative permeability set to " << _kn << " but it must be non-negative");
  if (_ks > 1)
    mooseError("Ks in BW relative permeability set to " << _ks << " but it must not be greater than 1");
  if (_ks <= _kn)
    mooseError("In BW relative permeability Kn set to " << _kn << " and Ks set to " << _ks << " but these must obey Ks > Kn");
  if (_c <= 1)
    mooseError("In BW relative permeability C set to " << _c << " but it must be greater than 1");
  _coef = (_ks - _kn)*(_c - 1); // shorthand coefficient
}


Real
RichardsRelPermBW::relperm(Real seff) const
{
  if (seff <= _sn) {
    return _kn;
  }

  if (seff >= _ss) {
    return _ks;
  }

  Real s_internal = (seff - _sn)/(_ss - _sn);
  Real krel = _kn + _coef*std::pow(s_internal, 2)/(_c - s_internal);

  return krel;
}


Real
RichardsRelPermBW::drelperm(Real seff) const
{
  if (seff <= _sn) {
    return 0.0;
  }

  if (seff >= _ss) {
    return 0.0;
  }

  Real s_internal = (seff - _sn)/(_ss - _sn);
  Real krelp = _coef*( 2.0*s_internal/(_c - s_internal) + std::pow(s_internal, 2)/std::pow(_c - s_internal, 2));
  return krelp/(_ss - _sn);
}


Real
RichardsRelPermBW::d2relperm(Real seff) const
{
  if (seff <= _sn) {
    return 0.0;
  }

  if (seff >= _ss) {
    return 0.0;
  }

  Real s_internal = (seff - _sn)/(_ss - _sn);
  Real krelpp = _coef*( 2.0/(_c - s_internal) + 4.0*s_internal/std::pow(_c - s_internal, 2) + 2*std::pow(s_internal, 2)/std::pow(_c - s_internal, 3) );
  return krelpp/std::pow(_ss - _sn, 2);
}

