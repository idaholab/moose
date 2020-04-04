//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  shifted van-Genuchten water effective saturation as a function of (Pwater, Pgas), and its derivs
//  wrt to that pressure
//
#include "RichardsSeff2waterVGshifted.h"

registerMooseObject("RichardsApp", RichardsSeff2waterVGshifted);

InputParameters
RichardsSeff2waterVGshifted::validParams()
{
  InputParameters params = RichardsSeff::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "al",
      "al > 0",
      "van-Genuchten alpha parameter.  Must be positive.   seff = (1 + "
      "(-al*(P0-P1-shift))^(1/(1-m)))^(-m) (then scaled to 0 to 1)");
  params.addRequiredRangeCheckedParam<Real>(
      "m",
      "m > 0 & m < 1",
      "van-Genuchten m parameter.  Must be between 0 and 1, and optimally "
      "should be set to >0.5   seff = (1 + "
      "(-al*(P0-P1-shift)^(1/(1-m)))^(-m) (then scaled to 0 to 1)");
  params.addRequiredRangeCheckedParam<Real>(
      "shift",
      "shift > 0",
      "Shift in capillary-pressure porepressure values.  Standard "
      "van-Genuchten Seff = Seff(Pwater-Pgas) is shifted to the right, and "
      "then scaled to 0<=Seff<=1.  This means that dS/dP>0 at S=1 which is "
      "useful to provide nonsingular Jacobians for small dt.");
  params.addClassDescription("Shifted van-Genuchten effective saturation as a function of (Pwater, "
                             "Pgas) suitable for use for the water phase in two-phase simulations. "
                             "    seff = (1 + (-al*(P0-p1-shift))^(1/(1-m)))^(-m), then scaled so "
                             "it runs between 0 and 1.");
  return params;
}

RichardsSeff2waterVGshifted::RichardsSeff2waterVGshifted(const InputParameters & parameters)
  : RichardsSeff(parameters),
    _al(getParam<Real>("al")),
    _m(getParam<Real>("m")),
    _shift(getParam<Real>("shift"))
{
  _scale = RichardsSeffVG::seff(-_shift, _al, _m);
}

Real
RichardsSeff2waterVGshifted::seff(std::vector<const VariableValue *> p, unsigned int qp) const
{
  Real negpc = (*p[0])[qp] - (*p[1])[qp];
  negpc = negpc - _shift;
  return std::min(RichardsSeffVG::seff(negpc, _al, _m) / _scale, 1.0);
}

void
RichardsSeff2waterVGshifted::dseff(std::vector<const VariableValue *> p,
                                   unsigned int qp,
                                   std::vector<Real> & result) const
{
  Real negpc = (*p[0])[qp] - (*p[1])[qp];
  negpc = negpc - _shift;
  result[0] = RichardsSeffVG::dseff(negpc, _al, _m) / _scale;
  result[1] = -result[0];
}

void
RichardsSeff2waterVGshifted::d2seff(std::vector<const VariableValue *> p,
                                    unsigned int qp,
                                    std::vector<std::vector<Real>> & result) const
{
  Real negpc = (*p[0])[qp] - (*p[1])[qp];
  negpc = negpc - _shift;
  result[0][0] = RichardsSeffVG::d2seff(negpc, _al, _m) / _scale;
  result[0][1] = -result[0][0];
  result[1][0] = -result[0][0];
  result[1][1] = result[0][0];
}
