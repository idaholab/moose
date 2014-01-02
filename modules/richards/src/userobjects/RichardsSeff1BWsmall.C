//  "Broadbridge-White" form of effective saturation for Kn small (P Broadbridge and I White ``Constant rate rainfall infiltration: A versatile nonlinear model 1. Analytic Solution'', Water Resources Research 24 (1988) 145-154)
//
#include "RichardsSeff1BWsmall.h"

template<>
InputParameters validParams<RichardsSeff1BWsmall>()
{
  InputParameters params = validParams<RichardsSeff>();
  params.addRequiredParam<Real>("Sn", "Low saturation.  This must be < Ss, and non-negative.  This is BW's initial effective saturation, below which effective saturation never goes in their simulations/models.  If Kn=0 then Sn is the immobile saturation.  This form of effective saturation is only correct for Kn small.");
  params.addParam<Real>("Ss", 1.0, "High saturation.  This must be > Sn and <= 1.  Effective saturation where porepressure = 0.  Effective saturation never exceeds this value in BW's simulations/models.");
  params.addRequiredParam<Real>("C", "BW's C parameter.  Must be > 1.  Typical value would be 1.05.");
  params.addRequiredParam<Real>("las", "BW's lambda_s parameter multiplied by (fluiddensity*gravity).  Must be > 0.  Typical value would be 1E5");
  params.addClassDescription("Broadbridge-white form of effective saturation for negligable Kn.  Then porepressure = -las*( (1-th)/th - (1/c)Ln((C-th)/((C-1)th))), for th = (Seff - Sn)/(Ss - Sn).  A Lambert-W function must be evaluated to express Seff in terms of porepressure, which can be expensive");
  return params;
}

RichardsSeff1BWsmall::RichardsSeff1BWsmall(const std::string & name, InputParameters parameters) :
  RichardsSeff(name, parameters),
  _sn(getParam<Real>("Sn")),
  _ss(getParam<Real>("Ss")),
  _c(getParam<Real>("C")),
  _las(getParam<Real>("las"))
{
  if (_sn < 0)
    mooseError("Sn in BW effective saturation set to " << _sn << " but it must be non-negative");
  if (_ss > 1)
    mooseError("Ss in BW effective saturation set to " << _ss << " but it must not be greater than 1");
  if (_ss <= _sn)
    mooseError("In BW effective saturation Sn set to " << _sn << " and Ss set to " << _ss << " but these must obey Ss > Sn");
  if (_c <= 1)
    mooseError("In BW effective saturation C set to " << _c << " but it must be greater than 1");
  if (_las <= 0)
    mooseError("In BW effective saturation las set to " << _las << " but it must be positive");
}
      


Real
RichardsSeff1BWsmall::seff(std::vector<VariableValue *> p, unsigned int qp) const
{
  Real pp = (*p[0])[qp];
  if (pp >= 0) return 1.0;

  Real x = (_c - 1)*std::exp(_c - 1 - _c*pp/_las);
  Real th = _c/(1 + LambertW(0, x)); // use branch 0 for positive x
  return _sn + (_ss - _sn)*th;
}

std::vector<Real>
RichardsSeff1BWsmall::dseff(std::vector<VariableValue *> p, unsigned int qp) const
{
  std::vector<Real> dseff_dp(1);
  dseff_dp[0] = 0.0;

  Real pp = (*p[0])[qp];
  if (pp >= 0) return dseff_dp;

  Real x = (_c - 1)*std::exp(_c - 1 - _c*pp/_las);
  Real lamw = LambertW(0, x);
  dseff_dp[0] = std::pow(_c, 2)/_las*lamw/std::pow(1 + lamw, 3);
  return dseff_dp;
}

std::vector<std::vector<Real> >
RichardsSeff1BWsmall::d2seff(std::vector<VariableValue *> p, unsigned int qp) const
{
  // create a dummy b that is 1x1 and zeroed
  std::vector<Real> a(1, 0);
  std::vector<std::vector <Real> > b(1, a);
  b[0][0] = 0.0;

  Real pp = (*p[0])[qp];
  if (pp >= 0) return b;

  Real x = (_c - 1)*std::exp(_c - 1 - _c*pp/_las);
  Real lamw = LambertW(0, x);
  b[0][0] = -std::pow(_c, 3)/std::pow(_las, 2)*lamw*(1 - 2*lamw)/std::pow(1 + lamw, 5);
  return b;
}
