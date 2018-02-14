#ifndef ADDCOMPONENTPHYSICSACTION_H
#define ADDCOMPONENTPHYSICSACTION_H

#include "RELAP7Action.h"

class AddComponentPhysicsAction;

template <>
InputParameters validParams<AddComponentPhysicsAction>();

class AddComponentPhysicsAction : public RELAP7Action
{
public:
  AddComponentPhysicsAction(InputParameters params);

  virtual void act();
};

#endif /* ADDCOMPONENTPHYSICSACTION_H */
