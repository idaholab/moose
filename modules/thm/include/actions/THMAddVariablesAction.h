#pragma once

#include "THMAction.h"

class THMAddVariablesAction;

template <>
InputParameters validParams<THMAddVariablesAction>();

class THMAddVariablesAction : public THMAction
{
public:
  THMAddVariablesAction(InputParameters params);

  virtual void act();
};
