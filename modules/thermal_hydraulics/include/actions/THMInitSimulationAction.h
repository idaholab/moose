#pragma once

#include "Action.h"

/**
 *
 */
class THMInitSimulationAction : public Action
{
public:
  THMInitSimulationAction(InputParameters parameters);

  virtual void act();

public:
  static InputParameters validParams();
};
