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
