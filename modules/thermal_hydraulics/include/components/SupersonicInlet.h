#pragma once

#include "FlowBoundary1Phase.h"

/**
 *
 */
class SupersonicInlet : public FlowBoundary1Phase
{
public:
  SupersonicInlet(const InputParameters & parameters);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

public:
  static InputParameters validParams();
};
