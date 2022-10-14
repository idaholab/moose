#pragma once

#include "SubChannel1PhaseProblem.h"
#include "QuadSubChannelMesh.h"

class LiquidWaterSubChannel1PhaseProblem;

/**
 * Steady state subchannel solver for 1-phase liquid water coolant
 */
class LiquidWaterSubChannel1PhaseProblem : public SubChannel1PhaseProblem
{
public:
  LiquidWaterSubChannel1PhaseProblem(const InputParameters & params);

protected:
  virtual double computeFrictionFactor(double Re) override;
  virtual double computeFrictionFactor(double Re, int i_ch) override;
  QuadSubChannelMesh & _subchannel_mesh;

public:
  static InputParameters validParams();
};
