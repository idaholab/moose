#pragma once

#include "FlowBoundary1Phase.h"

/**
 * Boundary condition with prescribed stagnation enthalpy and momentum for 1-phase flow channels
 */
class InletStagnationEnthalpyMomentum1Phase : public FlowBoundary1Phase
{
public:
  InletStagnationEnthalpyMomentum1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;

public:
  static InputParameters validParams();
};
