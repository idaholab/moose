#pragma once

#include "FlowBoundary1Phase.h"

/**
 * Boundary condition with prescribed mass flow rate and temperature for 1-phase flow channels
 */
class InletMassFlowRateTemperature1Phase : public FlowBoundary1Phase
{
public:
  InletMassFlowRateTemperature1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;

public:
  static InputParameters validParams();
};
