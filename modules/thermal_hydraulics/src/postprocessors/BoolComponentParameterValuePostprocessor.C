//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoolComponentParameterValuePostprocessor.h"
#include "ControllableParameter.h"
#include "InputParameterWarehouse.h"

registerMooseObject("ThermalHydraulicsApp", BoolComponentParameterValuePostprocessor);

InputParameters
BoolComponentParameterValuePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<std::string>("component", "The name of the component to be controlled.");
  params.addRequiredParam<std::string>(
      "parameter", "The name of the parameter in the component to be controlled.");
  params.addClassDescription(
      "Postprocessor for reading a boolean value from the control logic system.");
  return params;
}

BoolComponentParameterValuePostprocessor::BoolComponentParameterValuePostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _input_parameter_warehouse(_app.getInputParameterWarehouse()),
    _component_name(getParam<std::string>("component")),
    _param_name(getParam<std::string>("parameter")),
    _ctrl_param_name("component/" + _component_name + "/" + _param_name)
{
  std::vector<bool> values =
      _input_parameter_warehouse.getControllableParameterValues<bool>(_ctrl_param_name);
  if (values.size() == 0)
    paramError("component",
               "Either component '",
               _component_name,
               "' does not exist or parameter '",
               _param_name,
               "' does not exist in that component.");
}

void
BoolComponentParameterValuePostprocessor::initialize()
{
}

void
BoolComponentParameterValuePostprocessor::execute()
{
  std::vector<bool> values =
      _input_parameter_warehouse.getControllableParameterValues<bool>(_ctrl_param_name);

  _value = values[0];
}

Real
BoolComponentParameterValuePostprocessor::getValue()
{
  return _value;
}
