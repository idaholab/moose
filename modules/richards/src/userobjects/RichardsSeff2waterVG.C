/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  van-Genuchten water effective saturation as a function of (Pwater, Pgas), and its derivs wrt to
//  that pressure
//
#include "RichardsSeff2waterVG.h"

template <>
InputParameters
validParams<RichardsSeff2waterVG>()
{
  InputParameters params = validParams<RichardsSeff>();
  params.addRequiredRangeCheckedParam<Real>("al",
                                            "al > 0",
                                            "van-Genuchten alpha parameter.  Must "
                                            "be positive.  Single-phase VG seff = "
                                            "(1 + (-al*c)^(1/(1-m)))^(-m)");
  params.addRequiredRangeCheckedParam<Real>(
      "m",
      "m > 0 & m < 1",
      "van-Genuchten m parameter.  Must be between 0 and 1, and optimally "
      "should be set to >0.5   Single-phase VG seff = (1 + "
      "(-al*p)^(1/(1-m)))^(-m)");
  params.addClassDescription("van-Genuchten effective saturation as a function of (Pwater, Pgas) "
                             "suitable for use for the water phase in two-phase simulations.  With "
                             "Pc=Pgas-Pwater,   seff = (1 + (al*pc)^(1/(1-m)))^(-m)");
  return params;
}

RichardsSeff2waterVG::RichardsSeff2waterVG(const InputParameters & parameters)
  : RichardsSeff(parameters), _al(getParam<Real>("al")), _m(getParam<Real>("m"))
{
}

Real
RichardsSeff2waterVG::seff(std::vector<const VariableValue *> p, unsigned int qp) const
{
  Real negpc = (*p[0])[qp] - (*p[1])[qp];
  return RichardsSeffVG::seff(negpc, _al, _m);
}

void
RichardsSeff2waterVG::dseff(std::vector<const VariableValue *> p,
                            unsigned int qp,
                            std::vector<Real> & result) const
{
  Real negpc = (*p[0])[qp] - (*p[1])[qp];
  result[0] = RichardsSeffVG::dseff(negpc, _al, _m);
  result[1] = -result[0];
}

void
RichardsSeff2waterVG::d2seff(std::vector<const VariableValue *> p,
                             unsigned int qp,
                             std::vector<std::vector<Real>> & result) const
{
  Real negpc = (*p[0])[qp] - (*p[1])[qp];
  result[0][0] = RichardsSeffVG::d2seff(negpc, _al, _m);
  result[0][1] = -result[0][0];
  result[1][0] = -result[0][0];
  result[1][1] = result[0][0];
}
