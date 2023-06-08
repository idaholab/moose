//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetRealValueControl.h"

registerMooseObject("ThermalHydraulicsApp", SetRealValueControl);

InputParameters
SetRealValueControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params.addRequiredParam<std::string>("parameter", "The input parameter(s) to control");
  params.addRequiredParam<std::string>(
      "value", "The name of control data to be set into the input parameter.");
  params.addClassDescription("Control object that reads a Real value computed by the control logic "
                             "system and sets it into a specified MOOSE object parameter(s)");
  return params;
}

SetRealValueControl::SetRealValueControl(const InputParameters & parameters)
  : THMControl(parameters), _value(getControlData<Real>("value"))
{
}

void
SetRealValueControl::execute()
{
  setControllableValue<Real>("parameter", _value);
}
