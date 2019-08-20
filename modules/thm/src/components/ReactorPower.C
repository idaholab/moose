#include "ReactorPower.h"

template <>
InputParameters
validParams<ReactorPower>()
{
  InputParameters params = validParams<Component>();
  return params;
}

ReactorPower::ReactorPower(const InputParameters & parameters)
  : Component(parameters), _power_var_name(genName(name(), "power"))
{
}

void
ReactorPower::addVariables()
{
  _sim.addVariable(false, _power_var_name, FEType(FIRST, SCALAR));
}
