#pragma once

#include "Action.h"

class THMSetupMeshAction : public Action
{
public:
  THMSetupMeshAction(InputParameters params);

  virtual void act();

public:
  static InputParameters validParams();
};
