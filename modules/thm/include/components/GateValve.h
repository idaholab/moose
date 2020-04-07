#pragma once

#include "FlowJunction.h"

/**
 * Gate valve component
 */
class GateValve : public FlowJunction
{
public:
  GateValve(const InputParameters & params);

  virtual void check() const override;
  virtual void addMooseObjects() override;

protected:
  virtual void setupMesh() override;
  virtual void addMooseObjects1Phase() const;
  virtual void addMooseObjects2Phase() const;

public:
  static InputParameters validParams();
};
