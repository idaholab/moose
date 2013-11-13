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

#include "Restartable.h"
#include "SubProblem.h"

Restartable::Restartable(std::string name, InputParameters & parameters, std::string system_name) :
    _restartable_name(name),
    _restartable_params(&parameters),
    _restartable_system_name(system_name),
    _restartable_tid(parameters.isParamValid("_tid") ? parameters.get<THREAD_ID>("_tid") : 0),
    _restartable_subproblem(parameters.isParamValid("_subproblem") ? parameters.get<SubProblem *>("_subproblem") : (parameters.isParamValid("_fe_problem") ? (SubProblem*)parameters.get<FEProblem *>("_fe_problem") : NULL) )
{
}

Restartable::Restartable(std::string name, std::string system_name, SubProblem & fe_problem, THREAD_ID tid) :
    _restartable_name(name),
    _restartable_system_name(system_name),
    _restartable_tid(tid),
    _restartable_subproblem(&fe_problem)
{
}


Restartable::~Restartable()
{
}

void
Restartable::registerRestartableDataOnSubProblem(std::string name, RestartableDataValue * data, THREAD_ID tid)
{
  _restartable_subproblem->registerRestartableData(name, data, tid);
}
