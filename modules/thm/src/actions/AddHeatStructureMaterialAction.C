#include "AddHeatStructureMaterialAction.h"

registerMooseAction("THMApp", AddHeatStructureMaterialAction, "THM:add_heat_structure_material");

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
