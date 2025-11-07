//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConstantFunction.h"

registerKokkosFunction("MooseApp", KokkosConstantFunction);

InputParameters
KokkosConstantFunction::validParams()
{
  InputParameters params = FunctionBase::validParams();
  params.addClassDescription(
      "A function that returns a constant value as defined by an input parameter.");
  params.addParam<Real>("value", 0.0, "The constant value");
  params.declareControllable("value");
  return params;
}

KokkosConstantFunction::KokkosConstantFunction(const InputParameters & parameters)
  : FunctionBase(parameters), _value(getParam<Real>("value"))
{
}
