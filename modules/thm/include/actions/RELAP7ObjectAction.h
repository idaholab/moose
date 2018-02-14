#ifndef RELAP7OBJECTACTION_H
#define RELAP7OBJECTACTION_H

#include "MooseObjectAction.h"
#include "Simulation.h"

class RELAP7App;
class RELAP7ObjectAction;

template <>
InputParameters validParams<RELAP7ObjectAction>();

class RELAP7ObjectAction : public MooseObjectAction
{
public:
  RELAP7ObjectAction(InputParameters params);

protected:
  /// Simulation this action is part of
  Simulation & _simulation;
};

#endif /* RELAP7ACTION_H */
