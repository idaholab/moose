#pragma once

#include "Action.h"

class THMBuildMeshAction;

template <>
InputParameters validParams<THMBuildMeshAction>();

class THMBuildMeshAction : public Action
{
public:
  THMBuildMeshAction(InputParameters params);

  virtual void act();
};
