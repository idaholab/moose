#pragma once

#include "InterWrapper1PhaseProblem.h"

class LiquidWaterInterWrapper1PhaseProblem;

/**
 * Steady state subchannel solver for 1-phase liquid water coolant
 */
class LiquidWaterInterWrapper1PhaseProblem : public InterWrapper1PhaseProblem
{
public:
  LiquidWaterInterWrapper1PhaseProblem(const InputParameters & params);

protected:
  virtual double computeFrictionFactor(double Re) override;

public:
  static InputParameters validParams();
};
