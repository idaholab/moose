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

#include "UserDataInterface.h"
#include "UserData.h"
#include "Problem.h"

UserDataInterface::UserDataInterface(InputParameters & params) :
    _udi_problem(*params.get<Problem *>("_problem")),
    _udi_tid(params.have_parameter<THREAD_ID>("_tid") ? params.get<THREAD_ID>("_tid") : 0),
    _udi_params(params)
{
}

const UserData &
UserDataInterface::getUserData(const std::string & name)
{
  return _udi_problem.getUserData(_udi_params.get<std::string>(name), _udi_tid);
}

const UserData &
UserDataInterface::getUserDataByName(const std::string & name)
{
  return _udi_problem.getUserData(name, _udi_tid);
}
