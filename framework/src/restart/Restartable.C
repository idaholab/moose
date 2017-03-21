/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
  _restartable_subproblem = parameters.isParamValid("_subproblem")
                                ? parameters.get<SubProblem *>("_subproblem")
                                : (parameters.isParamValid("_fe_problem_base")
                                       ? parameters.get<FEProblemBase *>("_fe_problem_base")
                                       : (parameters.isParamValid("_fe_problem")
                                              ? parameters.get<FEProblem *>("_fe_problem")
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
