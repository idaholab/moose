//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "SolutionInvalidityOutput.h"
#include "MooseApp.h"
#include "MooseObjectParameterName.h"
#include "InputParameterWarehouse.h"
#include "ConsoleUtils.h"

registerMooseObject("MooseApp", SolutionInvalidityOutput);

InputParameters
SolutionInvalidityOutput::validParams()
{
  InputParameters params = Output::validParams();

  params.set<ExecFlagEnum>("execute_on") = {EXEC_FINAL};

  params.addParam<unsigned int>(
      "time_interval", 1, "The time step interval to report the solution invalidity occurances.");

  params.addClassDescription("Controls output of the time history of solution invalidity object");

  return params;
}

SolutionInvalidityOutput::SolutionInvalidityOutput(const InputParameters & parameters)
  : Output(parameters), _time_interval(getParam<unsigned int>("time_interval"))
{
}

bool
SolutionInvalidityOutput::shouldOutput()
{
  return _execute_on.isValueSet(_current_execute_flag);
}

void
SolutionInvalidityOutput::output()
{
  auto & solution_invalidity = _app.solutionInvalidity();

  if (solution_invalidity.hasInvalidSolution())
  {
    _console << '\n';
    solution_invalidity.printHistory(_console, _time_interval);

    _console << std::flush;
  }
}
