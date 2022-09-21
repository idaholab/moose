//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddFluidPropertiesDeprecatedAction.h"

registerMooseAction("FluidPropertiesApp",
                    AddFluidPropertiesDeprecatedAction,
                    "add_fluid_properties");

InputParameters
AddFluidPropertiesDeprecatedAction::validParams()
{
  return AddFluidPropertiesAction::validParams();
}

AddFluidPropertiesDeprecatedAction::AddFluidPropertiesDeprecatedAction(
    const InputParameters & params)
  : AddFluidPropertiesAction(params)
{
  mooseDeprecated(
      "The 'Modules/FluidProperties/*' syntax is deprecated. Use 'FluidProperties/*' instead.");
}
