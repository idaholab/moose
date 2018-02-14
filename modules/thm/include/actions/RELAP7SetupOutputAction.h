#ifndef RELAP7SETUPOUTPUTACTION_H
#define RELAP7SETUPOUTPUTACTION_H

#include "RELAP7Action.h"

class RELAP7SetupOutputAction;

template <>
InputParameters validParams<RELAP7SetupOutputAction>();

class RELAP7SetupOutputAction : public RELAP7Action
{
public:
  RELAP7SetupOutputAction(InputParameters params);

  virtual void act();
};

#endif /* RELAP7SETUPOUTPUTACTION_H */
