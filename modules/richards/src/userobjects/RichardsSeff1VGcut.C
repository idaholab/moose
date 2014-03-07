/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  "cut" van-Genuchten effective saturation as a function of pressure, and its derivs wrt p
//
#include "RichardsSeff1VGcut.h"

template<>
InputParameters validParams<RichardsSeff1VGcut>()
{
  InputParameters params = validParams<RichardsSeff1VG>();
  //params.addParam<Real>("al", "van-Genuchten alpha parameter.  Must be positive.  Single-phase  seff = (1 + (-al*p)^(1/(1-m)))^(-m) for p>p_cut, otherwse a linear function.");
  //params.addParam<Real>("m", "van-Genuchten m parameter.  Must be between 0 and 1, and optimally should be set to >0.5.  Single-phase    seff = (1 + (-al*p)^(1/(1-m)))^(-m) for p>p_cut, otherwise use a linear function.");
  params.addParam<Real>("p_cut", "cutoff in pressure.  Must be negative.  If p>p_cut then use van-Genuchten function.  Otherwise use a linear relationship which is chosen so the value and derivative match van-Genuchten at p=p_cut");
  params.addClassDescription("cut van-Genuchten effective saturation as a function of capillary pressure.  Single-phase  seff = (1 + (-al*p)^(1/(1-m)))^(-m) for p>p_cut, otherwise user a a linear relationship that is chosen so the value and derivative match van-Genuchten at p=p_cut.");
  return params;
}

RichardsSeff1VGcut::RichardsSeff1VGcut(const std::string & name, InputParameters parameters) :
  RichardsSeff1VG(name, parameters),
  //_al(getParam<Real>("al")),
  //_m(getParam<Real>("m")),
  _p_cut(getParam<Real>("p_cut")),
  _s_cut(0),
  _ds_cut(0)
{
  if (_al < 0)
    mooseError("The van-Genuchten alpha parameter in the cut effective saturation relationship is " << _al << " but this must be positive");
  if (_m <=0 || _m >= 1)
    mooseError("The van-Genuchten m parameter in the cut effective saturation relationship is " << _m << " but this must be between zero and one");
  if (_p_cut >= 0)
    mooseError("The p_cut parameter in the cut effective saturation relationship is " << _p_cut << " but it must be negative");
  _s_cut = RichardsSeffVG::seff(_p_cut, _al, _m);
  _ds_cut = RichardsSeffVG::dseff(_p_cut, _al, _m);
  Moose::out << "cut VG Seff has p_cut=" << _p_cut << " so seff_cut=" << _s_cut << " and seff=0 at p=" << -_s_cut/_ds_cut + _p_cut << "\n";
}



Real
RichardsSeff1VGcut::seff(std::vector<VariableValue *> p, unsigned int qp) const
{
  if ((*p[0])[qp] > _p_cut)
    {
      return RichardsSeff1VG::seff(p, qp);
    }
  else
    {
      Real seff_linear = _s_cut + _ds_cut*((*p[0])[qp] - _p_cut);
      //return (seff_linear > 0 ? seff_linear : 0); // andy isn't sure of this - might be useful to allow negative saturations
      return seff_linear;
    }
}

std::vector<Real>
RichardsSeff1VGcut::dseff(std::vector<VariableValue *> p, unsigned int qp) const
{
  if ((*p[0])[qp] > _p_cut)
    {
      return RichardsSeff1VG::dseff(p, qp);
    }
  else
    {
      //Real seff_linear = _s_cut + _ds_cut*((*p[0])[qp] - _p_cut);
      //return (seff_linear > 0 ? _ds_cut : 0);
      return std::vector<Real>(1, _ds_cut);
    }
}

std::vector<std::vector<Real> >
RichardsSeff1VGcut::d2seff(std::vector<VariableValue *> p, unsigned int qp) const
{
  if ((*p[0])[qp] > _p_cut)
    {
      return RichardsSeff1VG::d2seff(p, qp);
    }
  else
    {
      // create a dummy b that is 1x1 and zeroed
      std::vector<Real> a(1, 0);
      std::vector<std::vector <Real> > b(1, a);
      return b;
    }
}
