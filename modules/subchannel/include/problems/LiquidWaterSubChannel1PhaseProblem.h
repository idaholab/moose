#pragma once

#include "SubChannel1PhaseProblemBase.h"

class LiquidWaterSubChannel1PhaseProblem;

/**
 * Steady state subchannel solver for 1-phase liquid water coolant
 */
class LiquidWaterSubChannel1PhaseProblem : public SubChannel1PhaseProblemBase
{
public:
  LiquidWaterSubChannel1PhaseProblem(const InputParameters & params);

protected:
  virtual double computeFrictionFactor(double Re) override;

public:
  static InputParameters validParams();
};
