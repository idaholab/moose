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

registerMooseObject("MooseApp", SteadyExecutor);

InputParameters
SteadyExecutor::validParams()
{
  InputParameters params = Executor::validParams();
  params.addRequiredParam<std::vector<ExecutorName>>(
      "inner_executors", "The executors to run in between setup an output");
  params.addClassDescription(
      "A steady executor that does setup, runs inner executors, and then performs final output");
  return params;
}

SteadyExecutor::SteadyExecutor(const InputParameters & params)
  : Executor(params),
    _fe_problem(*params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
  for (const auto & name : getParam<std::vector<ExecutorName>>("inner_executors"))
    _inner_executors.push_back(&getExecutorByName<Executor>(name));
}

Executor::Result
SteadyExecutor::run()
{
  auto result = newResult();

  _fe_problem.initialSetup();
  for (auto * const inner : _inner_executors)
    result.record(inner->name(), inner->exec());

  _fe_problem.execute(EXEC_FINAL);
  _fe_problem.time() = 1;
  _fe_problem.timeStep() = 1;
  _fe_problem.outputStep(EXEC_FINAL);

  return result;
}
