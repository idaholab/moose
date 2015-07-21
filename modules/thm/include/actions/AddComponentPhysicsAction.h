#ifndef ADDCOMPONENTPHYSICSACTION_H
#define ADDCOMPONENTPHYSICSACTION_H

#include "R7Action.h"

class AddComponentPhysicsAction;

template<>
InputParameters validParams<AddComponentPhysicsAction>();

class AddComponentPhysicsAction : public R7Action
{
public:
  AddComponentPhysicsAction(InputParameters params);
  virtual ~AddComponentPhysicsAction();

  virtual void act();
};

#endif /* ADDCOMPONENTPHYSICSACTION_H */
