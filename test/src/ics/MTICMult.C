//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MTICMult.h"

registerMooseObject("MooseTestApp", MTICMult);

InputParameters
MTICMult::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredCoupledVar("var1", "Coupled variable");
  params.addRequiredParam<Real>("factor", "Some factor");

  return params;
}

MTICMult::MTICMult(const InputParameters & parameters)
  : InitialCondition(parameters), _var1(coupledValue("var1")), _factor(getParam<Real>("factor"))
{
}

MTICMult::~MTICMult() {}

Real
MTICMult::value(const Point & /*p*/)
{
  return _var1[_qp] * _factor;
}
