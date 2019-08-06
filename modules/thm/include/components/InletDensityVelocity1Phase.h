#pragma once

#include "FlowBoundary.h"

class InletDensityVelocity1Phase;

template <>
InputParameters validParams<InletDensityVelocity1Phase>();

/**
 * Boundary condition with prescribed density and velocity for 1-phase flow channels
 */
class InletDensityVelocity1Phase : public FlowBoundary
{
public:
  InletDensityVelocity1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;
};
