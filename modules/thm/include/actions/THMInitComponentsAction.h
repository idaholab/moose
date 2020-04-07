#pragma once

#include "Action.h"

/**
 * Initialize components
 */
class THMInitComponentsAction : public Action
{
public:
  THMInitComponentsAction(InputParameters parameters);

  virtual void act();

public:
  static InputParameters validParams();
};
