/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  van-Genuchten gas effective saturation as a function of (Pwater, Pgas), and its derivs wrt to that pressure
//
#include "RichardsSeff2gasVG.h"

template<>
InputParameters validParams<RichardsSeff2gasVG>()
{
  InputParameters params = validParams<RichardsSeff>();
  params.addParam<Real>("al", "van-Genuchten alpha parameter.  Must be positive.  Single-phase VG seff = (1 + (-al*c)^(1/(1-m)))^(-m)");
  params.addParam<Real>("m", "van-Genuchten m parameter.  Must be between 0 and 1, and optimally should be set to >0.5   Single-phase VG seff = (1 + (-al*p)^(1/(1-m)))^(-m)");
  params.addClassDescription("van-Genuchten effective saturation as a function of (Pwater, Pgas) suitable for use for the gas phase in two-phase simulations.  With Pc=Pgas-Pwater,   seff = 1 - (1 + (al*pc)^(1/(1-m)))^(-m)");
  return params;
}

RichardsSeff2gasVG::RichardsSeff2gasVG(const std::string & name, InputParameters parameters) :
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
RichardsSeff2gasVG::seff(std::vector<VariableValue *> p, unsigned int qp) const
{
  Real negpc = (*p[0])[qp] - (*p[1])[qp];
  return 1 - RichardsSeffVG::seff(negpc, _al, _m);
}

std::vector<Real>
RichardsSeff2gasVG::dseff(std::vector<VariableValue *> p, unsigned int qp) const
{
  Real negpc = (*p[0])[qp] - (*p[1])[qp];
  std::vector<Real> answer(2);
  answer[0] = -RichardsSeffVG::dseff(negpc, _al, _m);
  answer[1] = -answer[0];
  return answer;
}

std::vector<std::vector<Real> >
RichardsSeff2gasVG::d2seff(std::vector<VariableValue *> p, unsigned int qp) const
{
  Real negpc = (*p[0])[qp] - (*p[1])[qp];
  std::vector<std::vector<Real> > answer(2);
  answer[0].resize(2);
  answer[1].resize(2);
  answer[0][0] = -RichardsSeffVG::d2seff(negpc, _al, _m);
  answer[0][1] = -answer[0][0];
  answer[1][0] = -answer[0][0];
  answer[1][1] = answer[0][0];
  return answer;
}

