//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteElementUserObject.h"

InputParameters
DiscreteElementUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();

  // UOs of this type should not be executed by MOOSE, but only called directly by the user
  params.set<ExecFlagEnum>("execute_on") = EXEC_CUSTOM;
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

DiscreteElementUserObject::DiscreteElementUserObject(const InputParameters & parameters)
  : ElementUserObject(parameters)
{
}

void
DiscreteElementUserObject::initialize()
{
}

void
DiscreteElementUserObject::execute()
{
  mooseError("DiscreteElementUserObjects must be called explicitly from Materials");
}

void
DiscreteElementUserObject::finalize()
{
  mooseError("DiscreteElementUserObjects must be called explicitly from Materials");
}

void
DiscreteElementUserObject::threadJoin(const UserObject &)
{
  mooseError("DiscreteElementUserObjects must be called explicitly from Materials");
}
