#pragma once

#include "FlowBoundary.h"

class InletDensityVelocity;

template <>
InputParameters validParams<InletDensityVelocity>();

/**
 * Boundary condition with prescribed density and velocity for flow channels
 *
 */
class InletDensityVelocity : public FlowBoundary
{
public:
  InletDensityVelocity(const InputParameters & params);

protected:
  virtual void check() const override;
};
