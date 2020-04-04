//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantAux.h"

registerMooseObject("MooseApp", ConstantAux);

InputParameters
ConstantAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Creates a constant field in the domain.");
  params.addParam<Real>("value", 0.0, "Some constant value that can be read from the input file");
  params.declareControllable("value");
  return params;
}

ConstantAux::ConstantAux(const InputParameters & parameters)
  : AuxKernel(parameters), _value(getParam<Real>("value"))
{
}

Real
ConstantAux::computeValue()
{
  return _value;
}
