//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Function.h"

template <>
InputParameters
validParams<Function>()
{
  InputParameters params = validParams<MooseObject>();

  params.registerBase("Function");

  return params;
}

Function::Function(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    UserObjectInterface(this),
    Restartable(parameters, "Functions"),
    MeshChangedInterface(parameters),
    ScalarCoupleable(this)
{
}

Function::~Function() {}

Real
Function::value(Real /*t*/, const Point & /*p*/)
{
  return 0.0;
}

RealGradient
Function::gradient(Real /*t*/, const Point & /*p*/)
{
  return RealGradient(0, 0, 0);
}

Real
Function::timeDerivative(Real /*t*/, const Point & /*p*/)
{
  mooseError("timeDerivative method not defined for function ", name());
  return 0;
}

RealVectorValue
Function::vectorValue(Real /*t*/, const Point & /*p*/)
{
  return RealVectorValue(0, 0, 0);
}

Real
Function::integral()
{
  mooseError("Integral method not defined for function ", name());
  return 0;
}

Real
Function::average()
{
  mooseError("Average method not defined for function ", name());
  return 0;
}
