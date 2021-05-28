#pragma once

#include "SubChannel1PhaseProblemBase.h"

class LiquidMetalSubChannel1PhaseProblem;
class TriSubChannelMesh;
/**
 * Steady state subchannel solver for 1-phase liquid metal coolants
 */
class LiquidMetalSubChannel1PhaseProblem : public SubChannel1PhaseProblemBase
{
public:
  LiquidMetalSubChannel1PhaseProblem(const InputParameters & params);

protected:
  /**
   * computes the friction factor for the sodium coolant.
   * temporarily this is set to the friction factor correlation
   * given for water coolant.
   */
  virtual double computeFrictionFactor(double Re) override;
  /**
   * computeInletMassFlowDist corrects the inlet mass flow rate distribution
   * in order to satisfy the uniform inlet pressure condition, iteratively.
   */
  virtual void computeInletMassFlowDist();
  /**
   * solver with iterative option to enforce uniform inlet
   * pressure distribution option
   */
  virtual void externalSolve() override;
  /// average relative error in pressure drop of channels
  Real _dpz_error;
  TriSubChannelMesh & _tri_sch_mesh;

public:
  static InputParameters validParams();
};
