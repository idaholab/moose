#pragma once

#include "FlowBoundary1Phase.h"

/**
 * 1-phase inlet with all variables prescribed by functions.
 */
class InletFunction1Phase : public FlowBoundary1Phase
{
public:
  InletFunction1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

public:
  static InputParameters validParams();
};
