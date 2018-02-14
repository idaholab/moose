#ifndef RELAP7INITSIMULATIONACTION_H
#define RELAP7INITSIMULATIONACTION_H

#include "RELAP7Action.h"

class RELAP7InitSimulationAction;

template <>
InputParameters validParams<RELAP7InitSimulationAction>();

/**
 *
 */
class RELAP7InitSimulationAction : public RELAP7Action
{
public:
  RELAP7InitSimulationAction(InputParameters parameters);

  virtual void act();

protected:
};

#endif /* RELAP7INITSIMULATIONACTION_H */
