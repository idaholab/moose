#pragma once

#include "FlowBoundary.h"

class InletMassFlowRateTemperature;

template <>
InputParameters validParams<InletMassFlowRateTemperature>();

/**
 * Boundary condition with prescribed mass flow rate and temperature for flow channels
 *
 */
class InletMassFlowRateTemperature : public FlowBoundary
{
public:
  InletMassFlowRateTemperature(const InputParameters & params);

protected:
  virtual void check() const override;
};
