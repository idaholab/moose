#pragma once

#include "FlowBoundary.h"

class InletStagnationPressureTemperature;

template <>
InputParameters validParams<InletStagnationPressureTemperature>();

/**
 * Boundary condition with prescribed stagnation pressure and temperature for flow channels
 */
class InletStagnationPressureTemperature : public FlowBoundary
{
public:
  InletStagnationPressureTemperature(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;

  void setup1PhaseCG();
  void setup1PhaseRDG();

  void setup2PhaseCG();
  void setup2PhaseRDG();

  void setup2PhaseNCGCG();
};
