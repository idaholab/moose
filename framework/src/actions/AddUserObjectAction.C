//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddUserObjectAction.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<AddUserObjectAction>()
{
  return validParams<MooseObjectAction>();
}

AddUserObjectAction::AddUserObjectAction(InputParameters params) : MooseObjectAction(params) {}

void
AddUserObjectAction::act()
{
  _problem->addUserObject(_type, _name, _moose_object_pars);
}
