#ifndef RELAP7ACTION_H
#define RELAP7ACTION_H

#include "Action.h"
#include "Simulation.h"

class RELAP7App;
class RELAP7Action;

template <>
InputParameters validParams<RELAP7Action>();

class RELAP7Action : public Action
{
public:
  RELAP7Action(InputParameters params);

protected:
  /// Simulation this action is part of
  Simulation & _simulation;
};

#endif /* RELAP7ACTION_H */
