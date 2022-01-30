#include "AddHeatStructureMaterialAction.h"

registerMooseAction("ThermalHydraulicsApp",
                    AddHeatStructureMaterialAction,
                    "THM:add_heat_structure_material");

InputParameters
AddHeatStructureMaterialAction::validParams()
{
  return AddUserObjectAction::validParams();
}

AddHeatStructureMaterialAction::AddHeatStructureMaterialAction(InputParameters params)
  : AddUserObjectAction(params)
{
}
