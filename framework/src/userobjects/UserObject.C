//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UserObject.h"
#include "Assembly.h"

InputParameters
UserObject::validParams()
{
  InputParameters params = UserObjectBase::validParams();

  params.registerSystemAttributeName("UserObject");

  return params;
}

UserObject::UserObject(const InputParameters & parameters)
  : UserObjectBase(parameters), _coord_sys(_assembly.coordSystem())
{
}

void
UserObject::setPrimaryThreadCopy(UserObject * primary)
{
  if (!_primary_thread_copy && primary != this)
    _primary_thread_copy = primary;
}
