//  van-Genuchten effective saturation as a function of single pressure, and its derivs wrt to that pressure
//
#include "RichardsSeff1VG.h"

template<>
InputParameters validParams<RichardsSeff1VG>()
{
  InputParameters params = validParams<RichardsSeff>();
  params.addParam<Real>("al", "van-Genuchten alpha parameter.  Must be positive.  Single-phase VG seff = (1 + (-al*c)^(1/(1-m)))^(-m)");
  params.addParam<Real>("m", "van-Genuchten m parameter.  Must be between 0 and 1, and optimally should be set to >0.5   Single-phase VG seff = (1 + (-al*p)^(1/(1-m)))^(-m)");
  params.addClassDescription("van-Genuchten effective saturation as a function of pressure suitable for use in single-phase simulations..  seff = (1 + (-al*p)^(1/(1-m)))^(-m)");
  return params;
}

RichardsSeff1VG::RichardsSeff1VG(const std::string & name, InputParameters parameters) :
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
RichardsSeff1VG::seff(std::vector<VariableValue *> p, unsigned int qp) const
{
  Real n, seff;

  if ((*p[0])[qp] >= 0)
    {
      return 1.0;
    }
  else
    {
      n = 1.0/(1.0 - _m);
      seff = 1 + std::pow(-_al*(*p[0])[qp], n);
      return std::pow(seff, -_m);
    }
}

std::vector<Real>
RichardsSeff1VG::dseff(std::vector<VariableValue *> p, unsigned int qp) const
{
  Real n, inner, dinner_dp;
  
  if ((*p[0])[qp] >= 0)
    {
      return std::vector<Real>(1, 0);
    }
  else
    {
      n = 1.0/(1.0 - _m);
      inner = 1 + std::pow(-_al*(*p[0])[qp], n);
      dinner_dp = -n*_al*std::pow(-_al*(*p[0])[qp], n-1);
      std::vector<Real> dseff_dp(1);
      dseff_dp[0] = -_m*std::pow(inner, -_m - 1)*dinner_dp;
      return dseff_dp;
    }
}

std::vector<std::vector<Real> >
RichardsSeff1VG::d2seff(std::vector<VariableValue *> p, unsigned int qp) const
{
  Real n;
  Real inner, dinner_dp, d2inner_dp2;
  Real d2seff_dp2;

  // create a dummy b that is 1x1 and zeroed
  std::vector<Real> a(1, 0);
  std::vector<std::vector <Real> > b(1, a);
   
  if ((*p[0])[qp] >= 0)
    {
      return b;
    }
  else
    {
      n = 1.0/(1.0 - _m);
      inner = 1 + std::pow(-_al*(*p[0])[qp], n);
      dinner_dp = -n*_al*std::pow(-_al*(*p[0])[qp], n-1);
      d2inner_dp2 = n*(n-1)*_al*_al*std::pow(-_al*(*p[0])[qp], n-2);
      d2seff_dp2 = _m*(_m+1)*std::pow(inner, -_m - 2)*std::pow(dinner_dp, 2) - _m*std::pow(inner, -_m - 1)*d2inner_dp2;
      b[0][0] = d2seff_dp2;
      return b;
    }
}

