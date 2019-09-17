#include "TotalPowerBase.h"

template <>
InputParameters
validParams<TotalPowerBase>()
{
  InputParameters params = validParams<Component>();
  return params;
}

TotalPowerBase::TotalPowerBase(const InputParameters & parameters)
  : Component(parameters), _power_var_name(genName(name(), "power"))
{
}

void
TotalPowerBase::addVariables()
{
  _sim.addVariable(false, _power_var_name, FEType(FIRST, SCALAR));
}
