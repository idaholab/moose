#pragma once

#include "FlowBoundary.h"

class InletStagnationEnthalpyMomentum1Phase;

template <>
InputParameters validParams<InletStagnationEnthalpyMomentum1Phase>();

/**
 * Boundary condition with prescribed stagnation enthalpy and momentum for 1-phase flow channels
 */
class InletStagnationEnthalpyMomentum1Phase : public FlowBoundary
{
public:
  InletStagnationEnthalpyMomentum1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;
};
