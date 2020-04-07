#pragma once

#include "Action.h"

class AddComponentPhysicsAction : public Action
{
public:
  AddComponentPhysicsAction(InputParameters params);

  virtual void act();

public:
  static InputParameters validParams();
};
