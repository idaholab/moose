#pragma once

#include "Action.h"

class THMDebugAction;

template <>
InputParameters validParams<THMDebugAction>();

class THMDebugAction : public Action
{
public:
  THMDebugAction(InputParameters params);

  virtual void act();
};
