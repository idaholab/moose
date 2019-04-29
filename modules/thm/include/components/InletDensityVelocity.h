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

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;

  void setup1Phase();
  void setup2Phase();
  void setup2PhaseNCG();
};
