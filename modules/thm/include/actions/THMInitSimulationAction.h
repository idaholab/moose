#pragma once

#include "THMAction.h"

class THMInitSimulationAction;

template <>
InputParameters validParams<THMInitSimulationAction>();

/**
 *
 */
class THMInitSimulationAction : public THMAction
{
public:
  THMInitSimulationAction(InputParameters parameters);

  virtual void act();

protected:
};
