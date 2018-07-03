#include "AddHeatStructureMaterialAction.h"

registerMooseAction("RELAP7App",
                    AddHeatStructureMaterialAction,
                    "RELAP7:add_heat_structure_material");

template <>
InputParameters
validParams<AddHeatStructureMaterialAction>()
{
  return validParams<AddUserObjectAction>();
}

AddHeatStructureMaterialAction::AddHeatStructureMaterialAction(InputParameters params)
  : AddUserObjectAction(params)
{
}
