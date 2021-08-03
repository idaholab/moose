#pragma once

#include "MooseObjectAction.h"

/**
 * Action that creates SubChannel problem
 */
class SubChannelCreateProblemAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  SubChannelCreateProblemAction(const InputParameters & parameters);

  virtual void act() override;
};
