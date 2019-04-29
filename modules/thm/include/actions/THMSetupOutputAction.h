#pragma once

#include "THMAction.h"

class THMSetupOutputAction;

template <>
InputParameters validParams<THMSetupOutputAction>();

class THMSetupOutputAction : public THMAction
{
public:
  THMSetupOutputAction(InputParameters params);

  virtual void act();
};
