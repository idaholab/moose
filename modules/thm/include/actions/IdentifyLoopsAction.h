#pragma once

#include "Action.h"

/**
 * Identifies the component loops.
 */
class IdentifyLoopsAction : public Action
{
public:
  IdentifyLoopsAction(InputParameters parameters);

  virtual void act();

public:
  static InputParameters validParams();
};
