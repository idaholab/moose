#include "TerminateControl.h"
#include "Function.h"

registerMooseObject("RELAP7App", TerminateControl);

template <>
InputParameters
validParams<TerminateControl>()
{
  InputParameters params = validParams<RELAP7Control>();
  params.addRequiredParam<std::string>(
      "input", "The name of boolean control data indicating if simulation should be terminated.");
  params.addParam<bool>("throw_error", false, "Flag to throw an error on termination");
  params.addRequiredParam<std::string>("termination_message",
                                       "Message to use if termination occurs");
  return params;
}

TerminateControl::TerminateControl(const InputParameters & parameters)
  : RELAP7Control(parameters),

    _throw_error(getParam<bool>("throw_error")),
    _termination_message(getParam<std::string>("termination_message")),
    _terminate(getControlData<bool>("input"))
{
}

void
TerminateControl::execute()
{
  if (_terminate)
  {
    if (_throw_error)
      mooseError(_termination_message);
    else
    {
      _console << _termination_message << std::endl;
      _fe_problem.terminateSolve();
    }
  }
}
