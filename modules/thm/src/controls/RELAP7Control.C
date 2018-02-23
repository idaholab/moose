#include "RELAP7Control.h"

template <>
InputParameters
validParams<RELAP7Control>()
{
  InputParameters params = validParams<Control>();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
  return params;
}

RELAP7Control::RELAP7Control(const InputParameters & parameters)
  : Control(parameters),
    _sim(*_app.parameters().getCheckedPointerParam<Simulation *>("_sim"))
{
}
