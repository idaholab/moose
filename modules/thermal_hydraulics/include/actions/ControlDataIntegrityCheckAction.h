#pragma once

#include "Action.h"

/**
 * Action to trigger the check of control data integrity
 */
class ControlDataIntegrityCheckAction : public Action
{
public:
  ControlDataIntegrityCheckAction(InputParameters parameters);

  virtual void act();

public:
  static InputParameters validParams();
};
