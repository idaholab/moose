#pragma once

#include "Action.h"

class THMDebugAction : public Action
{
public:
  THMDebugAction(InputParameters params);

  virtual void act();

public:
  static InputParameters validParams();
};
