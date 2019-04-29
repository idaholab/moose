#pragma once

#include "AddUserObjectAction.h"

class AddHeatStructureMaterialAction;

template <>
InputParameters validParams<AddHeatStructureMaterialAction>();

class AddHeatStructureMaterialAction : public AddUserObjectAction
{
public:
  AddHeatStructureMaterialAction(InputParameters params);
};
