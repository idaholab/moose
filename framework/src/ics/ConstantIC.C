//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantIC.h"
#include "libmesh/point.h"

registerMooseObject("MooseApp", ConstantIC);

InputParameters
ConstantIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredParam<Real>("value", "The value to be set in IC");
  params.addClassDescription("Sets a constant field value.");
  return params;
}

ConstantIC::ConstantIC(const InputParameters & parameters)
  : InitialCondition(parameters), _value(getParam<Real>("value"))
{
}

Real
ConstantIC::value(const Point & /*p*/)
{
  return _value;
}
