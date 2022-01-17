#pragma once

#include "FlowBoundary1Phase.h"

/**
 * Adds the boundary terms resulting from an integration by parts of the
 * advection terms, using no external boundary data.
 */
class FreeBoundary1Phase : public FlowBoundary1Phase
{
public:
  FreeBoundary1Phase(const InputParameters & parameters);

  virtual void addMooseObjects() override;

public:
  static InputParameters validParams();
};
