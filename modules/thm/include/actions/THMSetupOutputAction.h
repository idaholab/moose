#ifndef THMSETUPOUTPUTACTION_H
#define THMSETUPOUTPUTACTION_H

#include "THMAction.h"

class THMSetupOutputAction;

template <>
InputParameters validParams<THMSetupOutputAction>();

class THMSetupOutputAction : public THMAction
{
public:
  THMSetupOutputAction(InputParameters params);

  virtual void act();
};

#endif /* THMSETUPOUTPUTACTION_H */
