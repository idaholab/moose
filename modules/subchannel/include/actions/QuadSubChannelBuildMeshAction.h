#pragma once

#include "Action.h"

/**
 * Action for building empty mesh object for quadrilateral geometry that is filled by mesh
 * generators
 */
class QuadSubChannelBuildMeshAction : public Action
{
public:
  QuadSubChannelBuildMeshAction(const InputParameters & params);

  virtual void act();

public:
  static InputParameters validParams();
};
