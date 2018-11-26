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
  params.addParam<bool>("latch",
                        false,
                        "Determines if the output of this control stays true after the trip went "
                        "from false to true.");
  return params;
}

UnitTripControl::UnitTripControl(const InputParameters & parameters)
  : RELAP7Control(parameters),
    _value(getControlData<Real>("input")),
    _threshold(getParam<Real>("threshold")),
    _state(declareComponentControlData<bool>("state")),
    _latch(getParam<bool>("latch")),
    _tripped(false)
{
}

void
UnitTripControl::execute()
{
  if (_latch && _tripped)
  {
    _state = true;
    return;
  }

  if (_value <= _threshold)
    _state = false;
  else
  {
    _state = true;
    _tripped = true;
  }
}
