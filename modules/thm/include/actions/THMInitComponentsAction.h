#pragma once

#include "Action.h"

class THMInitComponentsAction;

template <>
InputParameters validParams<THMInitComponentsAction>();

/**
 * Initialize components
 */
class THMInitComponentsAction : public Action
{
public:
  THMInitComponentsAction(InputParameters parameters);

  virtual void act();
};
