//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorFunctionIC.h"
#include "libmesh/point.h"

registerMooseObject("MooseApp", VectorFunctionIC);

template <>
InputParameters
validParams<VectorFunctionIC>()
{
  InputParameters params = validParams<VectorInitialCondition>();
  params.addRequiredParam<FunctionName>("function", "The initial condition vector function.");
  params.addClassDescription(
      "Sets component values for a vector field variable based on a vector function.");
  return params;
}

VectorFunctionIC::VectorFunctionIC(const InputParameters & parameters)
  : VectorInitialCondition(parameters), _function(getFunction("function"))
{
}

RealVectorValue
VectorFunctionIC::value(const Point & p)
{
  return _function.vectorValue(_t, p);
}
