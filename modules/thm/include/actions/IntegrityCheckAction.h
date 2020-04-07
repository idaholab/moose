#pragma once

#include "Action.h"

/**
 * Check the integrity of the simulation
 */
class IntegrityCheckAction : public Action
{
public:
  IntegrityCheckAction(InputParameters parameters);

  virtual void act();

public:
  static InputParameters validParams();
};
