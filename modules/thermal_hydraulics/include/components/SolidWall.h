#pragma once

#include "FlowConnection.h"

/**
 * A simple component for solid wall BC
 *
 * Deprecated
 */
class SolidWall : public FlowConnection
{
public:
  SolidWall(const InputParameters & params);

public:
  static InputParameters validParams();
};
