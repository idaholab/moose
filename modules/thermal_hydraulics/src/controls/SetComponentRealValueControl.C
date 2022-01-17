#include "SetComponentRealValueControl.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", SetComponentRealValueControl);

InputParameters
SetComponentRealValueControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params.addRequiredParam<std::string>("component", "The name of the component to be controlled.");
  params.addRequiredParam<std::string>(
      "parameter", "The name of the parameter in the component to be controlled.");
  params.addRequiredParam<std::string>("value",
                                       "The name of control data to be set in the component.");
  return params;
}

SetComponentRealValueControl::SetComponentRealValueControl(const InputParameters & parameters)
  : THMControl(parameters),
    _component_name(getParam<std::string>("component")),
    _param_name(getParam<std::string>("parameter")),
    _ctrl_param_name("component", _component_name, _param_name),
    _value(getControlData<Real>("value"))
{
}

void
SetComponentRealValueControl::execute()
{
  setControllableValueByName<Real>(_ctrl_param_name, _value);
}
