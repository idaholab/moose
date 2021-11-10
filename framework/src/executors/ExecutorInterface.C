//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExecutorInterface.h"

#include "FEProblem.h"
#include "MooseObject.h"
#include "NullExecutor.h"
#include "Executor.h"
#include "MooseApp.h"

InputParameters
ExecutorInterface::validParams()
{
  return emptyInputParameters();
}

ExecutorInterface::ExecutorInterface(const MooseObject * moose_object)
  : _ei_moose_object(moose_object), _ei_app(moose_object->getMooseApp())
{
}

Executor &
ExecutorInterface::getExecutor(const std::string & param_name) const
{
  auto & params = _ei_moose_object->parameters();

  if (!params.isParamValid(param_name))
    return *_ei_app.getNullExecutor();

  return _ei_app.getExecutor(_ei_moose_object->getParam<ExecutorName>(param_name));
}

Executor &
ExecutorInterface::getExecutorByName(const ExecutorName & executor_name) const
{
  return _ei_app.getExecutor(executor_name);
}
