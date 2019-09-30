#pragma once

#include "Action.h"

class THMInitSimulationAction;

template <>
InputParameters validParams<THMInitSimulationAction>();

/**
 *
 */
class THMInitSimulationAction : public Action
{
public:
  THMInitSimulationAction(InputParameters parameters);

  virtual void act();
};
