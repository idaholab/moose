#pragma once

#include "Action.h"

class AddComponentPhysicsAction;

template <>
InputParameters validParams<AddComponentPhysicsAction>();

class AddComponentPhysicsAction : public Action
{
public:
  AddComponentPhysicsAction(InputParameters params);

  virtual void act();
};
