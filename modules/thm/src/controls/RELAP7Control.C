#include "RELAP7Control.h"

template <>
InputParameters
validParams<RELAP7Control>()
{
  InputParameters params = validParams<Control>();
  params.set<MultiMooseEnum>("execute_on") = "initial timestep_begin";
  return params;
}

RELAP7Control::RELAP7Control(const InputParameters & parameters)
  : Control(parameters),
    Restartable(parameters, "RELAP7Controls"),
    _sim(*_app.parameters().get<Simulation *>("_sim"))
{
}
