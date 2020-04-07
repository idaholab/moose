#include "TotalPowerBase.h"

InputParameters
TotalPowerBase::validParams()
{
  InputParameters params = Component::validParams();
  return params;
}

TotalPowerBase::TotalPowerBase(const InputParameters & parameters)
  : Component(parameters), _power_var_name(genName(name(), "power"))
{
}

void
TotalPowerBase::addVariables()
{
  _sim.addSimVariable(false, _power_var_name, FEType(FIRST, SCALAR));
}
