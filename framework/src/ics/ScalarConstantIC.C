//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarConstantIC.h"

registerMooseObject("MooseApp", ScalarConstantIC);

defineLegacyParams(ScalarConstantIC);

InputParameters
ScalarConstantIC::validParams()
{
  InputParameters params = ScalarInitialCondition::validParams();
  params.set<Real>("value") = 0.0;
  return params;
}

ScalarConstantIC::ScalarConstantIC(const InputParameters & parameters)
  : ScalarInitialCondition(parameters), _value(getParam<Real>("value"))
{
}

Real
ScalarConstantIC::value()
{
  return _value;
}
