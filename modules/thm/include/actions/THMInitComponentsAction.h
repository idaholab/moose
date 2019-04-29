#pragma once

#include "THMAction.h"

class THMInitComponentsAction;

template <>
InputParameters validParams<THMInitComponentsAction>();

/**
 * Initialize components
 */
class THMInitComponentsAction : public THMAction
{
public:
  THMInitComponentsAction(InputParameters parameters);

  virtual void act();

protected:
};
