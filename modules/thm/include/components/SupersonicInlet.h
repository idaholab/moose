#pragma once

#include "FlowBoundary.h"

/**
 *
 */
class SupersonicInlet : public FlowBoundary
{
public:
  SupersonicInlet(const InputParameters & parameters);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

public:
  static InputParameters validParams();
};
