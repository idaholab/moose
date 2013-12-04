//  van-Genuchten effective saturation as a function of capillary pressure, and its derivs wrt pc
//
#include "RichardsSeffVG.h"

template<>
InputParameters validParams<RichardsSeffVG>()
{
  InputParameters params = validParams<RichardsSeff>();
  params.addParam<Real>("al", "van-Genuchten alpha parameter.  Must be positive.    seff = (1 + (al*pc)^(1/(1-m)))^(-m)");
  params.addParam<Real>("m", "van-Genuchten m parameter.  Must be between 0 and 1, and optimally should be set to >0.5    seff = (1 + (al*pc)^(1/(1-m)))^(-m)");
  params.addClassDescription("van-Genuchten effective saturation as a function of capillary pressure.  seff = (1 + (al*pc)^(1/(1-m)))^(-m)");
  return params;
}

RichardsSeffVG::RichardsSeffVG(const std::string & name, InputParameters parameters) :
  RichardsSeff(name, parameters),
  _al(getParam<Real>("al")),
  _m(getParam<Real>("m"))
{
  if (_al < 0) 
    mooseError("The van-Genuchten alpha parameter in the effective saturation relationship is " << _al << " but this must be positive");
  if (_m <=0 || _m >= 1)
    mooseError("The van-Genuchten m parameter in the effective saturation relationship is " << _m << " but this must be between zero and one");
}
      


Real
RichardsSeffVG::seff(Real pc) const
{
  Real n, seff;

  if (pc <= 0)
    {
      return 1.0;
    }
  else
    {
      n = 1.0/(1.0 - _m);
      seff = 1 + std::pow(_al*pc, n);
      return std::pow(seff, -_m);
    }
}

Real
RichardsSeffVG::dseff(Real pc) const
{
  Real n, inner, dinner_dpc, dseff_dpc;

  if (pc <= 0)
    {
      return 0.0;
    }
  else
    {
      n = 1.0/(1.0 - _m);
      inner = 1 + std::pow(_al*pc, n);
      dinner_dpc = n*_al*std::pow(_al*pc, n-1);
      dseff_dpc = -_m*std::pow(inner, -_m - 1)*dinner_dpc;
      return dseff_dpc;
    }
}

Real
RichardsSeffVG::d2seff(Real pc) const
{
  Real n;
  Real inner, dinner_dpc, d2inner_dpc2;
  Real dseff_dpc, d2seff_dpc2;


  if (pc <= 0)
    {
      return 0.0;
    }
  else
    {
      n = 1.0/(1.0 - _m);
      inner = 1 + std::pow(_al*pc, n);
      dinner_dpc = n*_al*std::pow(_al*pc, n-1);
      d2inner_dpc2 = n*(n-1)*_al*_al*std::pow(_al*pc, n-2);
      d2seff_dpc2 = _m*(_m+1)*std::pow(inner, -_m - 2)*std::pow(dinner_dpc, 2) - _m*std::pow(inner, -_m - 1)*d2inner_dpc2;
      return d2seff_dpc2;
    }
}

