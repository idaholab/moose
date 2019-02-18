#ifndef THMADDVARIABLESACTION_H
#define THMADDVARIABLESACTION_H

#include "THMAction.h"

class THMAddVariablesAction;

template <>
InputParameters validParams<THMAddVariablesAction>();

class THMAddVariablesAction : public THMAction
{
public:
  THMAddVariablesAction(InputParameters params);

  virtual void act();
};

#endif /* THMADDVARIABLESACTION_H */
