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

// MOOSE includes
#include "UserObjectInterface.h"
#include "DiscreteElementUserObject.h"
#include "InputParameters.h"

UserObjectInterface::UserObjectInterface(const MooseObject * moose_object)
  : _uoi_params(moose_object->parameters()),
    _uoi_feproblem(*_uoi_params.get<FEProblemBase *>("_fe_problem_base")),
    _uoi_tid(_uoi_params.have_parameter<THREAD_ID>("_tid") ? _uoi_params.get<THREAD_ID>("_tid") : 0)
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

bool
UserObjectInterface::isDiscreteUserObject(const UserObject & uo) const
{
  return dynamic_cast<const DiscreteElementUserObject *>(&uo) != NULL;
}
