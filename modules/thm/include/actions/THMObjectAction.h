#ifndef THMOBJECTACTION_H
#define THMOBJECTACTION_H

#include "MooseObjectAction.h"
#include "Simulation.h"

class THMApp;
class THMObjectAction;

template <>
InputParameters validParams<THMObjectAction>();

class THMObjectAction : public MooseObjectAction
{
public:
  THMObjectAction(InputParameters params);

protected:
  /// Simulation this action is part of
  Simulation & _simulation;
};

#endif /* THMACTION_H */
