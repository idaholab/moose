#pragma once

#include "Action.h"

/**
 * Action to setup output of vector-valued velocity
 */
class THMOutputVectorVelocityAction : public Action
{
public:
  THMOutputVectorVelocityAction(InputParameters params);

  virtual void act();

public:
  static InputParameters validParams();
};
