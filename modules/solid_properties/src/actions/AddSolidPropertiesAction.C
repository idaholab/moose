//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddSolidPropertiesAction.h"

registerMooseAction("SolidPropertiesApp", AddSolidPropertiesAction, "add_solid_properties");

InputParameters
AddSolidPropertiesAction::validParams()
{
  return AddUserObjectAction::validParams();
}

AddSolidPropertiesAction::AddSolidPropertiesAction(const InputParameters & params)
  : AddUserObjectAction(params)
{
}
