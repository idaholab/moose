//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddHeatStructureMaterialAction.h"

registerMooseAction("ThermalHydraulicsApp",
                    AddHeatStructureMaterialAction,
                    "THM:add_heat_structure_material");

InputParameters
AddHeatStructureMaterialAction::validParams()
{
  return AddUserObjectAction::validParams();
}

AddHeatStructureMaterialAction::AddHeatStructureMaterialAction(const InputParameters & params)
  : AddUserObjectAction(params)
{
}
