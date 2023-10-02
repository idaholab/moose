//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsBase.h"
#include "MooseUtils.h"

InputParameters
PhysicsBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Creates all the objects necessary to solve a particular physics");

  MooseEnum transient_options("true false same_as_executioner", "same_as_executioner");
  params.addParam<MooseEnum>(
      "transient", transient_options, "Whether the physics is to be solved as a transient");
  return params;
}

PhysicsBase::PhysicsBase(const InputParameters & parameters)
  : GeneralUserObject(parameters), _is_transient(getParam<bool>("transient"))
{
}
