#pragma once

#include "Action.h"

class THMPrintComponentLoopsAction : public Action
{
public:
  THMPrintComponentLoopsAction(InputParameters params);

  virtual void act();

public:
  static InputParameters validParams();
};
