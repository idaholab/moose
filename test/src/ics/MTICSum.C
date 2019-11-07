//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MTICSum.h"

registerMooseObject("MooseTestApp", MTICSum);

InputParameters
MTICSum::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredCoupledVar("var1", "First variable");
  params.addRequiredCoupledVar("var2", "Second variable");

  return params;
}

MTICSum::MTICSum(const InputParameters & parameters)
  : InitialCondition(parameters), _var1(coupledValue("var1")), _var2(coupledValue("var2"))
{
}

MTICSum::~MTICSum() {}

Real
MTICSum::value(const Point & /*p*/)
{
  return _var1[_qp] + _var2[_qp] + 3;
}
