#pragma once

#include "FlowBoundary.h"

class InletStagnationPressureTemperature1Phase;

template <>
InputParameters validParams<InletStagnationPressureTemperature1Phase>();

/**
 * Boundary condition with prescribed stagnation pressure and temperature for 1-phase flow channels
 */
class InletStagnationPressureTemperature1Phase : public FlowBoundary
{
public:
  InletStagnationPressureTemperature1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;

  void setup1PhaseCG();
  void setup1PhaseRDG();
};
