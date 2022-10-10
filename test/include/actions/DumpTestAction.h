#pragma once

#include "Action.h"

/**
 * Action that creates a ton of different system objects to test the DumpObjectsProblem
 */
class DumpTestAction : public Action
{
public:
  static InputParameters validParams();

  DumpTestAction(const InputParameters & parameters);

  virtual void act() override;
};
