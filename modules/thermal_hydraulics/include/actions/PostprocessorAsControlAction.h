#pragma once

#include "MooseObjectAction.h"

/**
 * This action creates a control value named the same as the postprocessor being added
 *
 * This allows people to use the postprocessor values directly within the control system.
 */
class PostprocessorAsControlAction : public MooseObjectAction
{
public:
  PostprocessorAsControlAction(InputParameters params);

  virtual void act();

public:
  static InputParameters validParams();
};
