#pragma once

#include "FlowBoundary1Phase.h"

/**
 * Boundary condition with prescribed pressure for 1-phase flow channels
 */
class Outlet1Phase : public FlowBoundary1Phase
{
public:
  Outlet1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

public:
  static InputParameters validParams();
};
