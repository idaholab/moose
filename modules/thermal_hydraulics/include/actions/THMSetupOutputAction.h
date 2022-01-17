#pragma once

#include "Action.h"

class THMSetupOutputAction : public Action
{
public:
  THMSetupOutputAction(InputParameters params);

  virtual void act();

public:
  static InputParameters validParams();
};
