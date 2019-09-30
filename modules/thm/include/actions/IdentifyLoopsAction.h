#pragma once

#include "Action.h"

class IdentifyLoopsAction;

template <>
InputParameters validParams<IdentifyLoopsAction>();

/**
 * Identifies the component loops.
 */
class IdentifyLoopsAction : public Action
{
public:
  IdentifyLoopsAction(InputParameters parameters);

  virtual void act();
};
