#pragma once

#include "THMAction.h"

class IdentifyLoopsAction;

template <>
InputParameters validParams<IdentifyLoopsAction>();

/**
 * Identifies the component loops.
 */
class IdentifyLoopsAction : public THMAction
{
public:
  IdentifyLoopsAction(InputParameters parameters);

  virtual void act();
};
