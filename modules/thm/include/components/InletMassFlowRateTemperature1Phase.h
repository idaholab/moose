#pragma once

#include "FlowBoundary.h"

class InletMassFlowRateTemperature1Phase;

template <>
InputParameters validParams<InletMassFlowRateTemperature1Phase>();

/**
 * Boundary condition with prescribed mass flow rate and temperature for 1-phase flow channels
 */
class InletMassFlowRateTemperature1Phase : public FlowBoundary
{
public:
  InletMassFlowRateTemperature1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;

  void setup1PhaseCG();
  void setup1PhaseRDG();
};
