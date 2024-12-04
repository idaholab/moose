//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TerminateChainControl.h"

registerMooseObject("MooseApp", TerminateChainControl);

InputParameters
TerminateChainControl::validParams()
{
  InputParameters params = ChainControl::validParams();

  params.addClassDescription(
      "Terminates the simulation when a boolean chain control data has a certain value.");

  params.addRequiredParam<std::string>(
      "input", "Boolean control data indicating if the simulation should be terminated");
  params.addParam<bool>("terminate_on_true",
                        true,
                        "If set to 'true', termination occurs if the input has a value of 'true'; "
                        "else termination occurs if the input has a value of 'false'");
  params.addParam<bool>("throw_error",
                        false,
                        "Flag to throw an error on termination; else just signal the problem to "
                        "terminate the solve.");
  params.addRequiredParam<std::string>("termination_message",
                                       "Message to use if termination occurs");
  return params;
}

TerminateChainControl::TerminateChainControl(const InputParameters & parameters)
  : ChainControl(parameters),

    _terminate_on_true(getParam<bool>("terminate_on_true")),
    _throw_error(getParam<bool>("throw_error")),
    _termination_message(getParam<std::string>("termination_message")),
    _input(getChainControlData<bool>("input"))
{
}

void
TerminateChainControl::execute()
{
  if (_terminate_on_true && _input)
    terminate();

  if (!_terminate_on_true && !_input)
    terminate();
}

void
TerminateChainControl::terminate()
{
  if (_throw_error)
    mooseError(_termination_message);
  else
  {
    _console << _termination_message << std::endl;
    _fe_problem.terminateSolve();
  }
}
