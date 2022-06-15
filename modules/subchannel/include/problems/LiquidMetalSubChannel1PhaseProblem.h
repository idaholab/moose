#pragma once

#include "SubChannel1PhaseProblem.h"

class LiquidMetalSubChannel1PhaseProblem;
class TriSubChannelMesh;
/**
 * Steady state subchannel solver for 1-phase liquid metal coolants
 */
class LiquidMetalSubChannel1PhaseProblem : public SubChannel1PhaseProblem
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
  /// Function that computes the heat flux added by the duct
  virtual Real computeAddedHeatDuct(unsigned int i_ch, unsigned int iz);
  /// computeDP(int iz) is defined/overridden in order to use the friction factor for sodium
  virtual void computeDP(int iblock) override;
  /// computeMassFlowForDPDZ(double dpdz, int i_ch) and enforceUniformDPDZAtInlet()
  /// are overriden to define the sodium friction factor
  virtual double computeMassFlowForDPDZ(double dpdz, int i_ch);
  virtual void enforceUniformDPDZAtInlet();
  virtual void computeWijPrime(int iblock) override;
  ///
  virtual void computeh(int iblock) override;
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
  Real _outer_channels;

public:
  static InputParameters validParams();
};
