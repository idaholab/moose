//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  InputParameters params = AddUserObjectAction::validParams();
  params.addClassDescription("Adds HeatStructureMaterials to the Problem");
  return params;
}

AddHeatStructureMaterialAction::AddHeatStructureMaterialAction(const InputParameters & params)
  : AddUserObjectAction(params)
{
}
