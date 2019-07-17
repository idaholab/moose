#pragma once

#include "FlowJunction.h"

class GateValve;

template <>
InputParameters validParams<GateValve>();

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
  virtual void addMooseObjects1Phase() const;
  virtual void addMooseObjects2Phase() const;
};
