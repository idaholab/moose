#pragma once

#include "Action.h"

class THMAddVariablesAction : public Action
{
public:
  THMAddVariablesAction(InputParameters params);

  virtual void act();

public:
  static InputParameters validParams();
};
