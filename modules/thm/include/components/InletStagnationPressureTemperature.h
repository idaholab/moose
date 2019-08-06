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

protected:
  virtual void check() const override;
};
