#include "UnitTripControl.h"

registerMooseObject("RELAP7App", UnitTripControl);

template <>
InputParameters
validParams<UnitTripControl>()
{
  InputParameters params = validParams<RELAP7Control>();
  params.addRequiredParam<std::string>(
      "input", "The name of the control data that we compare against a threshold.");
  params.addRequiredParam<Real>("threshold", "The threshold that will trip this control.");
  return params;
}

UnitTripControl::UnitTripControl(const InputParameters & parameters)
  : RELAP7Control(parameters),
    _value(getControlData<Real>("input")),
    _threshold(getParam<Real>("threshold")),
    _output(declareControlData<bool>("output"))
{
}

void
UnitTripControl::execute()
{
  if (_value <= _threshold)
    _output = false;
  else
    _output = true;
}
