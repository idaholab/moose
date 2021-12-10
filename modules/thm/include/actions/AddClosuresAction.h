#pragma once

#include "MooseObjectAction.h"

/**
 * Adds a closures object
 */
class AddClosuresAction : public MooseObjectAction
{
public:
  AddClosuresAction(InputParameters params);

  virtual void act() override;

  static InputParameters validParams();
};
