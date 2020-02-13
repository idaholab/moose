#pragma once

#include "FlowBoundary.h"

class InletVelocityTemperature1Phase;

template <>
InputParameters validParams<InletVelocityTemperature1Phase>();

/**
 * Boundary condition with prescribed velocity and temperature for 1-phase flow channels
 */
class InletVelocityTemperature1Phase : public FlowBoundary
{
public:
  InletVelocityTemperature1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  /// True to allow the flow to reverse, otherwise false
  bool _reversible;

  void setup1PhaseRDG();
};
