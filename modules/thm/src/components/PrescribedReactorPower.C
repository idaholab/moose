#include "PrescribedReactorPower.h"

registerMooseObject("THMApp", PrescribedReactorPower);

template <>
InputParameters
validParams<PrescribedReactorPower>()
{
  InputParameters params = validParams<TotalPowerBase>();
  params.addRequiredParam<Real>("power", "Total power [W]");
  return params;
}

PrescribedReactorPower::PrescribedReactorPower(const InputParameters & parameters)
  : TotalPowerBase(parameters)
{
  logError("Deprecated component, use 'type = TotalPower' instead.");
}
