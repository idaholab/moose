#pragma once

#include "FlowJunction.h"

/**
 * Gate valve component for 1-phase flow
 */
class GateValve1Phase : public FlowJunction
{
public:
  GateValve1Phase(const InputParameters & params);

  virtual void check() const override;
  virtual void addMooseObjects() override;

protected:
  virtual void setupMesh() override;

public:
  static InputParameters validParams();
};
