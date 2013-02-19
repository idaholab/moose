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

#include "UserObjectInterface.h"
#include "FEProblem.h"

UserObjectInterface::UserObjectInterface(InputParameters & params) :
    _uoi_feproblem(*params.get<FEProblem *>("_fe_problem")),
    _uoi_tid(params.have_parameter<THREAD_ID>("_tid") ? params.get<THREAD_ID>("_tid") : 0),
    _uoi_params(params)
{
}

const UserObject &
UserObjectInterface::getUserObjectBase(const std::string & name)
{
  return _uoi_feproblem.getUserObjectBase(_uoi_params.get<UserObjectName>(name));
}

const UserObject &
UserObjectInterface::getUserObjectBaseByName(const std::string & name)
{
  return _uoi_feproblem.getUserObjectBase(name);
}
