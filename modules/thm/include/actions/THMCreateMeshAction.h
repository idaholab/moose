#pragma once

#include "Action.h"

class THMCreateMeshAction : public Action
{
public:
  THMCreateMeshAction(InputParameters params);

  virtual void act();

public:
  static InputParameters validParams();
};
