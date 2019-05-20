#include "THMControl.h"

template <>
InputParameters
validParams<THMControl>()
{
  InputParameters params = validParams<Control>();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

THMControl::THMControl(const InputParameters & parameters)
  : Control(parameters), _sim(*_app.parameters().getCheckedPointerParam<Simulation *>("_sim"))
{
}
