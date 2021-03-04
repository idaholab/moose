#pragma once

#include "FlowBoundary1Phase.h"

/**
 * Component for solid wall BC for 1-phase flow
 */
class SolidWall1Phase : public FlowBoundary1Phase
{
public:
  SolidWall1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

public:
  static InputParameters validParams();
};
