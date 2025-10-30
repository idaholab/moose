//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddSCMClosureAction.h"

registerMooseAction("SubChannelApp", AddSCMClosureAction, "add_scm_closure");

InputParameters
AddSCMClosureAction::validParams()
{
  InputParameters params = AddUserObjectAction::validParams();
  params.addClassDescription(
      "Add a closure (friction, heat transfer coefficient, etc) to the problem.");
  return params;
}

AddSCMClosureAction::AddSCMClosureAction(const InputParameters & params)
  : AddUserObjectAction(params)
{
}
