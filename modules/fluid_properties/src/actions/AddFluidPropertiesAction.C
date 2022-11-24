//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddFluidPropertiesAction.h"

registerMooseAction("FluidPropertiesApp", AddFluidPropertiesAction, "add_fluid_properties");

InputParameters
AddFluidPropertiesAction::validParams()
{
  return AddUserObjectAction::validParams();
}

AddFluidPropertiesAction::AddFluidPropertiesAction(const InputParameters & params)
  : AddUserObjectAction(params)
{
}
