#pragma once

#include "THMAction.h"

class THMSetupMeshAction;

template <>
InputParameters validParams<THMSetupMeshAction>();

class THMSetupMeshAction : public THMAction
{
public:
  THMSetupMeshAction(InputParameters params);

  virtual void act();
};
