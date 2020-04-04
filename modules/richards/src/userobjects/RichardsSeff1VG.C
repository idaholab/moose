//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  van-Genuchten effective saturation as a function of single pressure, and its derivs wrt to that
//  pressure
//
#include "RichardsSeff1VG.h"

registerMooseObject("RichardsApp", RichardsSeff1VG);

InputParameters
RichardsSeff1VG::validParams()
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
  params.addClassDescription("van-Genuchten effective saturation as a function of pressure "
                             "suitable for use in single-phase simulations..  seff = (1 + "
                             "(-al*p)^(1/(1-m)))^(-m)");
  return params;
}

RichardsSeff1VG::RichardsSeff1VG(const InputParameters & parameters)
  : RichardsSeff(parameters), _al(getParam<Real>("al")), _m(getParam<Real>("m"))
{
}

Real
RichardsSeff1VG::seff(std::vector<const VariableValue *> p, unsigned int qp) const
{
  return RichardsSeffVG::seff((*p[0])[qp], _al, _m);
}

void
RichardsSeff1VG::dseff(std::vector<const VariableValue *> p,
                       unsigned int qp,
                       std::vector<Real> & result) const
{
  result[0] = RichardsSeffVG::dseff((*p[0])[qp], _al, _m);
}

void
RichardsSeff1VG::d2seff(std::vector<const VariableValue *> p,
                        unsigned int qp,
                        std::vector<std::vector<Real>> & result) const
{
  result[0][0] = RichardsSeffVG::d2seff((*p[0])[qp], _al, _m);
}
