#ifndef GLOBALSIMPARAMACTION_H
#define GLOBALSIMPARAMACTION_H

#include "THMAction.h"

class GlobalSimParamAction;

template <>
InputParameters validParams<GlobalSimParamAction>();

/**
 * Action that sets all global params for a simulation
 */
class GlobalSimParamAction : public THMAction
{
public:
  GlobalSimParamAction(InputParameters parameters);

  virtual void act();

protected:
};

#endif /* GLOBALSIMPARAMACTION_H */
