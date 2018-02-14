#ifndef RELAP7ADDVARIABLESACTION_H
#define RELAP7ADDVARIABLESACTION_H

#include "RELAP7Action.h"

class RELAP7AddVariablesAction;

template <>
InputParameters validParams<RELAP7AddVariablesAction>();

class RELAP7AddVariablesAction : public RELAP7Action
{
public:
  RELAP7AddVariablesAction(InputParameters params);

  virtual void act();
};

#endif /* RELAP7ADDVARIABLESACTION_H */
