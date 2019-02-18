#ifndef THMINITSIMULATIONACTION_H
#define THMINITSIMULATIONACTION_H

#include "THMAction.h"

class THMInitSimulationAction;

template <>
InputParameters validParams<THMInitSimulationAction>();

/**
 *
 */
class THMInitSimulationAction : public THMAction
{
public:
  THMInitSimulationAction(InputParameters parameters);

  virtual void act();

protected:
};

#endif /* THMINITSIMULATIONACTION_H */
