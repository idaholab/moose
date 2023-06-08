//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TerminateControl.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", TerminateControl);

InputParameters
TerminateControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params.addClassDescription(
      "Terminates the simulation when a THMControl boolean data becomes true");
  params.addRequiredParam<std::string>(
      "input", "The name of boolean control data indicating if simulation should be terminated.");
  params.addParam<bool>("throw_error", false, "Flag to throw an error on termination");
  params.addRequiredParam<std::string>("termination_message",
                                       "Message to use if termination occurs");
  return params;
}

TerminateControl::TerminateControl(const InputParameters & parameters)
  : THMControl(parameters),

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
