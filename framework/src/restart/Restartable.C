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
#include "FEProblem.h"

Restartable::Restartable(std::string name, InputParameters & parameters, std::string system_name) :
    _restartable_name(name),
    _restartable_params(parameters),
    _restartable_system_name(system_name),
    _restartable_tid(parameters.isParamValid("_tid") ? parameters.get<THREAD_ID>("_tid") : 0),
    _restartable_feproblem(parameters.isParamValid("_fe_problem") ? parameters.get<FEProblem *>("_fe_problem") : NULL)
{
}


Restartable::~Restartable()
{
}

void
Restartable::registerRestartableDataOnFEProblem(std::string name, RestartableDataValue * data, THREAD_ID tid)
{
  _restartable_feproblem->registerRestartableData(name, data, tid);
}
