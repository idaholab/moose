//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "FEProblemBase.h"

registerMooseObject("MooseApp", SolutionInvalidityOutput);

InputParameters
SolutionInvalidityOutput::validParams()
{
  InputParameters params = Output::validParams();

  params.set<ExecFlagEnum>("execute_on") = {EXEC_FINAL, EXEC_FAILED};

  params.addParam<unsigned int>("solution_invalidity_timestep_interval",
                                1,
                                "The number of time steps to group together in the table reporting "
                                "the solution invalidity occurrences.");

  params.addClassDescription("Controls output of the time history of solution invalidity object");

  return params;
}

SolutionInvalidityOutput::SolutionInvalidityOutput(const InputParameters & parameters)
  : Output(parameters),
    _timestep_interval(getParam<unsigned int>("solution_invalidity_timestep_interval")),
    _solution_invalidity(_app.solutionInvalidity())
{
}

bool
SolutionInvalidityOutput::shouldOutput()
{
  // solver could have failed before any iteration completed, thus before a sync
  // Note: if this happens in other cases, we should just sync if solutionInvalidity is not synced
  if (_problem_ptr->getCurrentExecuteOnFlag() == EXEC_FAILED)
    _solution_invalidity.syncIteration();
  return Output::shouldOutput() && (_solution_invalidity.hasInvalidSolution());
}

void
SolutionInvalidityOutput::output()
{
  if (isParamSetByUser("solution_invalidity_timestep_interval"))
    mooseInfo("Set Outputs/solution_invalidity_history=false to silence the default Solution "
              "Invalid Warnings History "
              "table above.");
  _console << '\n';
  _solution_invalidity.printHistory(_console, _timestep_interval);
  _console << std::flush;
}
