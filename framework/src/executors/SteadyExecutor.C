//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SteadyExecutor.h"
#include "FEProblemBase.h"
#include "MooseApp.h"

registerMooseObject("MooseApp", SteadyExecutor);

InputParameters
SteadyExecutor::validParams()
{
  InputParameters params = Executor::validParams();
  params.addClassDescription("Executor for steady-state simulations that runs inner executors "
                             "between problem setup and output");
  params.addRequiredParam<std::vector<ExecutorName>>(
      "inner_executors", "The executors to run, in order, between problem setup and output");
  return params;
}

SteadyExecutor::SteadyExecutor(const InputParameters & parameters) : Executor(parameters)
{
  for (const auto & name : getParam<std::vector<ExecutorName>>("inner_executors"))
    _inner_executors.push_back(&getExecutorByName<Executor>(name));
}

void
SteadyExecutor::init()
{
  _fe_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _fe_problem.initialSetup();
  _app.initialSetupExecutors();
}

Executor::Result
SteadyExecutor::run()
{
  Result & result = newResult();

  // Steady-state output uses time step 0 for the initial condition and 1 for the solution
  _fe_problem.timeStep() = 0;
  _fe_problem.time() = 0;
  _fe_problem.outputStep(EXEC_INITIAL);

  _fe_problem.advanceState();
  _fe_problem.timeStep() = 1;
  _fe_problem.time() = 1;
  _fe_problem.timestepSetup();
  _fe_problem.execute(EXEC_TIMESTEP_BEGIN);

  for (auto * const inner : _inner_executors)
    if (!result.record(inner->name(), inner->exec()))
      break;

  if (result.convergedAll())
  {
    _fe_problem.onTimestepEnd();
    _fe_problem.execute(EXEC_TIMESTEP_END);
    _fe_problem.outputStep(EXEC_TIMESTEP_END);
  }

  _fe_problem.postExecute();
  _fe_problem.execute(EXEC_FINAL);
  _fe_problem.outputStep(EXEC_FINAL);

  return result;
}
