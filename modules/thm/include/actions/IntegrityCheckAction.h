#pragma once

#include "Action.h"

class IntegrityCheckAction;

template <>
InputParameters validParams<IntegrityCheckAction>();

/**
 * Check the integrity of the simulation
 */
class IntegrityCheckAction : public Action
{
public:
  IntegrityCheckAction(InputParameters parameters);

  virtual void act();

protected:
};
