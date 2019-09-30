#pragma once

#include "Action.h"

class THMCreateMeshAction;

template <>
InputParameters validParams<THMCreateMeshAction>();

class THMCreateMeshAction : public Action
{
public:
  THMCreateMeshAction(InputParameters params);

  virtual void act();
};
