#pragma once

#include "THMAction.h"

class GlobalSimParamAction;

template <>
InputParameters validParams<GlobalSimParamAction>();

/**
 * Action that sets all global params for a simulation
 */
class GlobalSimParamAction : public THMAction
{
public:
  GlobalSimParamAction(InputParameters parameters);

  virtual void act();

protected:
};
