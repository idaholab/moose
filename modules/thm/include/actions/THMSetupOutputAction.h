#pragma once

#include "Action.h"

class THMSetupOutputAction;

template <>
InputParameters validParams<THMSetupOutputAction>();

class THMSetupOutputAction : public Action
{
public:
  THMSetupOutputAction(InputParameters params);

  virtual void act();
};
