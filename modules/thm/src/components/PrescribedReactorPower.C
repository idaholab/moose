#include "PrescribedReactorPower.h"

registerMooseObject("THMApp", PrescribedReactorPower);

InputParameters
PrescribedReactorPower::validParams()
{
  InputParameters params = TotalPowerBase::validParams();
  params.addRequiredParam<Real>("power", "Total power [W]");
  return params;
}

PrescribedReactorPower::PrescribedReactorPower(const InputParameters & parameters)
  : TotalPowerBase(parameters)
{
  logError("Deprecated component, use 'type = TotalPower' instead.");
}
