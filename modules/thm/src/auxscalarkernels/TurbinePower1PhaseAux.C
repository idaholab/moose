#include "TurbinePower1PhaseAux.h"

registerMooseObject("THMApp", TurbinePower1PhaseAux);

InputParameters
TurbinePower1PhaseAux::validParams()
{
  InputParameters params = ConstantScalarAux::validParams();
  params.addRequiredParam<bool>("on", "Flag determining if turbine is operating or not");
  params.addClassDescription("Computes turbine power for 1-phase flow");
  params.declareControllable("on");
  return params;
}

TurbinePower1PhaseAux::TurbinePower1PhaseAux(const InputParameters & parameters)
  : ConstantScalarAux(parameters), _on(getParam<bool>("on"))
{
}

Real
TurbinePower1PhaseAux::computeValue()
{
  if (_on)
    return _value;
  else
    return 0.;
}
