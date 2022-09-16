#pragma once

#include "Action.h"

/**
 * Action for building empty mesh object for quadrilateral geometry that is filled by mesh
 * generators
 */
class QuadInterWrapperBuildMeshAction : public Action
{
public:
  QuadInterWrapperBuildMeshAction(const InputParameters & params);

  virtual void act();

public:
  static InputParameters validParams();
};
