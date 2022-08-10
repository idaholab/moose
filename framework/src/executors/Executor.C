//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Executor.h"
#include "MooseApp.h"
#include "FEProblem.h"

#include "ExecFlagRegistry.h"

InputParameters
Executor::validParams()
{
  InputParameters params = Executioner::validParams();

  params.addParam<ExecFlagType>(
      "begin_exec_flag", EXEC_NONE, "exec flag associated with the beginning of this executor");
  params.addParam<ExecFlagType>(
      "end_exec_flag", EXEC_NONE, "exec flag associated with the end of this executor");
  params.registerBase("Executor");
  return params;
}

Executor::Executor(const InputParameters & parameters)
  : Executioner(parameters, false),
    ExecutorInterface(this),
    _begin_flag(getParam<ExecFlagType>("begin_exec_flag")),
    _end_flag(getParam<ExecFlagType>("end_exec_flag"))
{
  if (!parameters.isParamSetByUser("begin_exec_flag"))
    _begin_flag = registerExecFlag("exec_" + _name + "_begin");
  if (!parameters.isParamSetByUser("end_exec_flag"))
    _end_flag = registerExecFlag("exec_" + _name + "_end");
}

Executor::Result
Executor::exec()
{
  _fe_problem.executeAllObjects(_begin_flag);
  auto result = run();
  _fe_problem.executeAllObjects(_end_flag);

  _result = result;
  return result;
}
