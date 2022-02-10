//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleTurbinePowerAux.h"

registerMooseObject("ThermalHydraulicsApp", SimpleTurbinePowerAux);

InputParameters
SimpleTurbinePowerAux::validParams()
{
  InputParameters params = ConstantScalarAux::validParams();
  params.addRequiredParam<bool>("on", "Flag determining if turbine is operating or not");
  params.addClassDescription("Computes turbine power for 1-phase flow");
  params.declareControllable("on");
  return params;
}

SimpleTurbinePowerAux::SimpleTurbinePowerAux(const InputParameters & parameters)
  : ConstantScalarAux(parameters), _on(getParam<bool>("on"))
{
}

Real
SimpleTurbinePowerAux::computeValue()
{
  if (_on)
    return _value;
  else
    return 0.;
}
