//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BooleanValueTestAux.h"

registerMooseObject("ThermalHydraulicsTestApp", BooleanValueTestAux);

InputParameters
BooleanValueTestAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<bool>("value", "Boolean value to convert into real.");
  params.addClassDescription(
      "Takes a boolean value and converts it into a Real value (0 for false, 1 for true)");
  params.declareControllable("value");
  return params;
}

BooleanValueTestAux::BooleanValueTestAux(const InputParameters & parameters)
  : AuxKernel(parameters), _value(getParam<bool>("value"))
{
}

Real
BooleanValueTestAux::computeValue()
{
  if (_value)
    return 1.;
  else
    return 0.;
}
