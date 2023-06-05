//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetComponentBoolValueControl.h"

registerMooseObject("ThermalHydraulicsApp", SetComponentBoolValueControl);

InputParameters
SetComponentBoolValueControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params.addRequiredParam<std::string>("component", "The name of the component to be controlled.");
  params.addRequiredParam<std::string>(
      "parameter", "The name of the parameter in the component to be controlled.");
  params.addRequiredParam<std::string>("value",
                                       "The name of control data to be set in the component.");
  params.addClassDescription(
      "Control to set a boolean value of a component parameter with control data boolean");
  return params;
}

SetComponentBoolValueControl::SetComponentBoolValueControl(const InputParameters & parameters)
  : THMControl(parameters),
    _component_name(getParam<std::string>("component")),
    _param_name(getParam<std::string>("parameter")),
    _ctrl_param_name("component", _component_name, _param_name),
    _value(getControlData<bool>("value"))
{
}

void
SetComponentBoolValueControl::execute()
{
  setControllableValueByName<bool>(_ctrl_param_name, _value);
}
