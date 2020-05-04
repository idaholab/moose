#include "SimpleTurbinePowerAux.h"

registerMooseObject("THMApp", SimpleTurbinePowerAux);

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
