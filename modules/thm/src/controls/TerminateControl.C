#include "TerminateControl.h"
#include "Function.h"

template <>
InputParameters
validParams<TerminateControl>()
{
  InputParameters params = validParams<RELAP7Control>();
  params.addRequiredParam<std::string>(
      "input", "The name of boolean control data indicating if simulation should be terminated.");
  return params;
}

TerminateControl::TerminateControl(const InputParameters & parameters)
  : RELAP7Control(parameters), _terminate(getControlData<bool>("input"))
{
}

void
TerminateControl::execute()
{
  if (_terminate)
    _fe_problem.terminateSolve();
}
