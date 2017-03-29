#include "AddHeatStructureMaterialAction.h"

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
