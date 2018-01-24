//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InputParameters.h"
#include "Restartable.h"
#include "SubProblem.h"
#include "FEProblem.h"

Restartable::Restartable(const InputParameters & parameters,
                         std::string system_name,
                         SubProblem * subproblem)
  : _restartable_name(parameters.get<std::string>("_object_name")),
    _restartable_params(&parameters),
    _restartable_system_name(system_name),
    _restartable_tid(parameters.isParamValid("_tid") ? parameters.get<THREAD_ID>("_tid") : 0)
{
  _restartable_subproblem =
      parameters.isParamValid("_subproblem")
          ? parameters.getCheckedPointerParam<SubProblem *>("_subproblem")
          : (parameters.isParamValid("_fe_problem_base")
                 ? parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")
                 : (parameters.isParamValid("_fe_problem")
                        ? parameters.getCheckedPointerParam<FEProblem *>("_fe_problem")
                        : subproblem));
}

Restartable::Restartable(const std::string & name,
                         std::string system_name,
                         SubProblem & fe_problem,
                         THREAD_ID tid)
  : _restartable_name(name),
    _restartable_system_name(system_name),
    _restartable_tid(tid),
    _restartable_subproblem(&fe_problem)
{
}

void
Restartable::registerRestartableDataOnSubProblem(std::string name,
                                                 RestartableDataValue * data,
                                                 THREAD_ID tid)
{
  _restartable_subproblem->registerRestartableData(name, data, tid);
}

void
Restartable::registerRecoverableDataOnSubProblem(std::string name)
{
  _restartable_subproblem->registerRecoverableData(name);
}
