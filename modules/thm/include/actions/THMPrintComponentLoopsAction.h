#pragma once

#include "Action.h"

class THMPrintComponentLoopsAction;

template <>
InputParameters validParams<THMPrintComponentLoopsAction>();

class THMPrintComponentLoopsAction : public Action
{
public:
  THMPrintComponentLoopsAction(InputParameters params);

  virtual void act();
};
