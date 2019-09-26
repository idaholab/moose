#pragma once

#include "Action.h"

class THMAddVariablesAction;

template <>
InputParameters validParams<THMAddVariablesAction>();

class THMAddVariablesAction : public Action
{
public:
  THMAddVariablesAction(InputParameters params);

  virtual void act();
};
