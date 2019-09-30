#pragma once

#include "Action.h"

class THMSetupMeshAction;

template <>
InputParameters validParams<THMSetupMeshAction>();

class THMSetupMeshAction : public Action
{
public:
  THMSetupMeshAction(InputParameters params);

  virtual void act();
};
