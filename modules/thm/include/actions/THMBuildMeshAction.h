#pragma once

#include "Action.h"

class THMBuildMeshAction : public Action
{
public:
  THMBuildMeshAction(InputParameters params);

  virtual void act();

public:
  static InputParameters validParams();
};
