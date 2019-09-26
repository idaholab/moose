#pragma once

#include "Action.h"

class ControlDataIntegrityCheckAction;

template <>
InputParameters validParams<ControlDataIntegrityCheckAction>();

/**
 * Action to trigger the check of control data integrity
 */
class ControlDataIntegrityCheckAction : public Action
{
public:
  ControlDataIntegrityCheckAction(InputParameters parameters);

  virtual void act();

protected:
};
