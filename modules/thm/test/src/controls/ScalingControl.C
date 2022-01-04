#include "ScalingControl.h"

registerMooseObject("THMTestApp", ScalingControl);

InputParameters
ScalingControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params.addRequiredParam<Real>("scale", "Scaling factor");
  params.addRequiredParam<Real>("initial", "Initial value");
  params.addClassDescription("Control that multiplies old value by a scalar. Used for testing time "
                             "dependent control values.");
  return params;
}

ScalingControl::ScalingControl(const InputParameters & parameters)
  : THMControl(parameters),
    _scale(getParam<Real>("scale")),
    _initial(getParam<Real>("initial")),
    _value(declareComponentControlData<Real>("value")),
    _value_old(getComponentControlDataOld<Real>("value"))
{
  _value = _initial;
}

void
ScalingControl::execute()
{
  _value = _scale * _value_old;
}
