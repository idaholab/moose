#pragma once

#include "THMAction.h"

class AddComponentPhysicsAction;

template <>
InputParameters validParams<AddComponentPhysicsAction>();

class AddComponentPhysicsAction : public THMAction
{
public:
  AddComponentPhysicsAction(InputParameters params);

  virtual void act();
};
