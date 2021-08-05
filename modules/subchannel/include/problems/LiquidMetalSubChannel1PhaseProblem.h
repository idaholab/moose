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
   * Computes the axial friction factor for the sodium coolant and
   * for each subchannel.
   * Upgraded Cheng-Todreas Correlation (2018).
   */
  virtual double computeFrictionFactor(double Re, int i_ch, Real S, Real w_perim, Real Dh_i);
  /// computeFrictionFactor(double Re) is currently not used for sodium coolant
  virtual double computeFrictionFactor(double Re) override;
  /// computeDP(int iz) is defined/overridden in order to use the friction factor for sodium
  virtual void computeDP(int iz) override;
  /// computeMassFlowForDPDZ(double dpdz, int i_ch) and enforceUniformDPDZAtInlet()
  /// are overriden to define the sodium friction factor
  virtual double computeMassFlowForDPDZ(double dpdz, int i_ch) override;
  virtual void enforceUniformDPDZAtInlet() override;
  /// turbulent-mixing cross flow model
  virtual void computeWijPrime(int iz) override;
  ///
  virtual void computeH(int iz) override;
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
