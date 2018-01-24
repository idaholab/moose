//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "UserObjectInterface.h"
#include "DiscreteElementUserObject.h"
#include "InputParameters.h"

UserObjectInterface::UserObjectInterface(const MooseObject * moose_object)
  : _uoi_params(moose_object->parameters()),
    _uoi_feproblem(*_uoi_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
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
