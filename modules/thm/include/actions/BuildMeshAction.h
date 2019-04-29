#pragma once

#include "THMAction.h"

class BuildMeshAction;

template <>
InputParameters validParams<BuildMeshAction>();

class BuildMeshAction : public THMAction
{
public:
  BuildMeshAction(InputParameters params);

  virtual void act();
};
