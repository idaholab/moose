#pragma once

#include "InputParameters.h"
#include "Action.h"

class AnisotropyInterfaceMaterialAction : public Action
{
public:
  static InputParameters validParams();

  AnisotropyInterfaceMaterialAction(const InputParameters & params);

  virtual void act();
};
