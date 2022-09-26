//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddSolidPropertiesDeprecatedAction.h"

registerMooseAction("SolidPropertiesApp",
                    AddSolidPropertiesDeprecatedAction,
                    "add_solid_properties");

InputParameters
AddSolidPropertiesDeprecatedAction::validParams()
{
  return AddSolidPropertiesAction::validParams();
}

AddSolidPropertiesDeprecatedAction::AddSolidPropertiesDeprecatedAction(
    const InputParameters & params)
  : AddSolidPropertiesAction(params)
{
  mooseDeprecated(
      "The syntax 'Modules/SolidProperties/*' is deprecated. Use 'SolidProperties/*' instead.");
}
