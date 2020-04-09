//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  van-Genuchten gas effective saturation as a function of (Pwater, Pgas), and its derivs wrt to
//  that pressure
//
#include "RichardsSeff2gasVG.h"

registerMooseObject("RichardsApp", RichardsSeff2gasVG);

InputParameters
RichardsSeff2gasVG::validParams()
{
  InputParameters params = RichardsSeff::validParams();
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
                             "suitable for use for the gas phase in two-phase simulations.  With "
                             "Pc=Pgas-Pwater,   seff = 1 - (1 + (al*pc)^(1/(1-m)))^(-m)");
  return params;
}

RichardsSeff2gasVG::RichardsSeff2gasVG(const InputParameters & parameters)
  : RichardsSeff(parameters), _al(getParam<Real>("al")), _m(getParam<Real>("m"))
{
}

Real
RichardsSeff2gasVG::seff(std::vector<const VariableValue *> p, unsigned int qp) const
{
  Real negpc = (*p[0])[qp] - (*p[1])[qp];
  return 1 - RichardsSeffVG::seff(negpc, _al, _m);
}

void
RichardsSeff2gasVG::dseff(std::vector<const VariableValue *> p,
                          unsigned int qp,
                          std::vector<Real> & result) const
{
  Real negpc = (*p[0])[qp] - (*p[1])[qp];
  result[0] = -RichardsSeffVG::dseff(negpc, _al, _m);
  result[1] = -result[0];
}

void
RichardsSeff2gasVG::d2seff(std::vector<const VariableValue *> p,
                           unsigned int qp,
                           std::vector<std::vector<Real>> & result) const
{
  Real negpc = (*p[0])[qp] - (*p[1])[qp];
  result[0][0] = -RichardsSeffVG::d2seff(negpc, _al, _m);
  result[0][1] = -result[0][0];
  result[1][0] = -result[0][0];
  result[1][1] = result[0][0];
}
