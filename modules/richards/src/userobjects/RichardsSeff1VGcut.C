//  "cut" van-Genuchten effective saturation as a function of capillary pressure, and its derivs wrt pc
//
#include "RichardsSeffVG1.h"

template<>
InputParameters validParams<RichardsSeffVG1>()
{
  InputParameters params = validParams<RichardsSeffVG>();
  params.addParam<Real>("al", "van-Genuchten alpha parameter.  Must be positive.    seff = (1 + (al*pc)^(1/(1-m)))^(-m) for pc<p_cut, otherwse a linear function.");
  params.addParam<Real>("m", "van-Genuchten m parameter.  Must be between 0 and 1, and optimally should be set to >0.5    seff = (1 + (al*pc)^(1/(1-m)))^(-m) for pc<p_cut, otherwise use a linear function.");
  params.addParam<Real>("p_cut", "cutoff in pressure.  Must be positive.  If pc<p_cut then use van-Genuchten function.  Otherwise use a linear relationship which is chosen so the value and derivative match van-Genuchten at pc=p_cut");
  params.addClassDescription("cut van-Genuchten effective saturation as a function of capillary pressure.  seff = (1 + (al*pc)^(1/(1-m)))^(-m) for pc<p_cut, otherwise user a a linear relationship that is chosen so the value and derivative match van-Genuchten at pc=p_cut.");
  return params;
}

RichardsSeffVG1::RichardsSeffVG1(const std::string & name, InputParameters parameters) :
  RichardsSeffVG(name, parameters),
  _al(getParam<Real>("al")),
  _m(getParam<Real>("m")),
  _p_cut(getParam<Real>("p_cut")),
  _s_cut(0),
  _ds_cut(0)
{
  if (_al < 0) 
    mooseError("The van-Genuchten alpha parameter in the cut effective saturation relationship is " << _al << " but this must be positive");
  if (_m <=0 || _m >= 1)
    mooseError("The van-Genuchten m parameter in the cut effective saturation relationship is " << _m << " but this must be between zero and one");
  if (_p_cut <= 0)
    mooseError("The p_cut parameter in the cut effective saturation relationship is " << _p_cut << " but it must be positive");
  _s_cut = RichardsSeffVG::seff(_p_cut);
  _ds_cut = RichardsSeffVG::dseff(_p_cut);
  std::cout << "cut VG Seff has p_cut=" << _p_cut << " so seff_cut=" << _s_cut << " and seff=0 at pc=" << -_s_cut/_ds_cut + _p_cut << "\n";
}
      


Real
RichardsSeffVG1::seff(Real pc) const
{
  if (pc <= 0)
    {
      return 1.0;
    }
  else if (pc < _p_cut)
    {
      return RichardsSeffVG::seff(pc);
    }
  else
    {
      Real seff_linear = _s_cut + _ds_cut*(pc - _p_cut);
      //return (seff_linear > 0 ? seff_linear : 0); // andy isn't sure of this - might be useful to allow negative saturations
      return seff_linear;
    }
}

Real
RichardsSeffVG1::dseff(Real pc) const
{
  if (pc <= 0)
    {
      return 0.0;
    }
  else if (pc < _p_cut)
    {
      return RichardsSeffVG::dseff(pc);
    }
  else
    {
      Real seff_linear = _s_cut + _ds_cut*(pc - _p_cut);
      //return (seff_linear > 0 ? _ds_cut : 0);
      return _ds_cut;
    }
}

Real
RichardsSeffVG1::d2seff(Real pc) const
{
  if (pc <= 0)
    {
      return 0.0;
    }
  else if (pc < _p_cut)
    {
      return RichardsSeffVG::d2seff(pc);
    }
  else
    {
      return 0.0;
    }
}    
