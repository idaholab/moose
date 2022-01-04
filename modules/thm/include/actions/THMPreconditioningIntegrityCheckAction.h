#pragma once

#include "Action.h"

/**
 * Action to trigger the integrity check of preconditioner
 */
class THMPreconditioningIntegrityCheckAction : public Action
{
public:
  THMPreconditioningIntegrityCheckAction(InputParameters parameters);

  virtual void act();

public:
  static InputParameters validParams();
};
