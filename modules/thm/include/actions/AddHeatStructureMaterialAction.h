#pragma once

#include "AddUserObjectAction.h"

class AddHeatStructureMaterialAction : public AddUserObjectAction
{
public:
  AddHeatStructureMaterialAction(InputParameters params);

public:
  static InputParameters validParams();
};
