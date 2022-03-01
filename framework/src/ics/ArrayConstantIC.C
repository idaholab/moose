//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayConstantIC.h"

#include "libmesh/point.h"

registerMooseObject("MooseApp", ArrayConstantIC);

InputParameters
ArrayConstantIC::validParams()
{
  InputParameters params = ArrayInitialCondition::validParams();
  params.addRequiredParam<RealEigenVector>("value", "The values to be set in IC");
  params.addClassDescription("Sets constant component values for an array field variable.");
  return params;
}

ArrayConstantIC::ArrayConstantIC(const InputParameters & parameters)
  : ArrayInitialCondition(parameters), _value(getParam<RealEigenVector>("value"))
{
  if (_var.count() != _value.size())
    mooseError("'value' size is inconsistent to the number of components of the array variable");
}

RealEigenVector
ArrayConstantIC::value(const Point & /*p*/)
{
  return _value;
}
