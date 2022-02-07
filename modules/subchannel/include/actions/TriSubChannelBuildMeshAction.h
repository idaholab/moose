#pragma once

#include "Action.h"

/**
 * Action for building empty mesh object for hexagonal geometry that is filled by mesh
 * generators
 */
class TriSubChannelBuildMeshAction : public Action
{
public:
  TriSubChannelBuildMeshAction(InputParameters params);

  virtual void act();

public:
  static InputParameters validParams();
};
