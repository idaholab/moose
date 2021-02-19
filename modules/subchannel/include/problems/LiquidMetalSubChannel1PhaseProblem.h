#pragma once

#include "SubChannel1PhaseProblemBase.h"

class LiquidMetalSubChannel1PhaseProblem;

/**
 * Steady state subchannel solver for 1-phase liquid metal coolants
 */
class LiquidMetalSubChannel1PhaseProblem : public SubChannel1PhaseProblemBase
{
public:
  LiquidMetalSubChannel1PhaseProblem(const InputParameters & params);

protected:
  virtual double computeFrictionFactor(double Re) override;

public:
  static InputParameters validParams();
};
