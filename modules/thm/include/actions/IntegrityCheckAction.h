#pragma once

#include "THMAction.h"

class IntegrityCheckAction;

template <>
InputParameters validParams<IntegrityCheckAction>();

/**
 * Check the integrity of the simulation
 */
class IntegrityCheckAction : public THMAction
{
public:
  IntegrityCheckAction(InputParameters parameters);

  virtual void act();

protected:
};
