//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantScalarAux.h"

registerMooseObject("MooseApp", ConstantScalarAux);

InputParameters
ConstantScalarAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addClassDescription("Sets an auxiliary field variable to a controllable constant value.");
  params.addRequiredParam<Real>("value", "The value to be set to the scalar variable.");
  params.declareControllable("value");
  return params;
}

ConstantScalarAux::ConstantScalarAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters), _value(getParam<Real>("value"))
{
}

Real
ConstantScalarAux::computeValue()
{
  return _value;
}
