#pragma once

#include "ExternalProblem.h"
#include "SubChannelApp.h"
#include "SubChannelMeshBase.h"
#include "SolutionHandle.h"
#include "SinglePhaseFluidProperties.h"

class SubChannel1PhaseProblemBase;

/**
 * Base class for the 1-phase steady state sub channel solver.
 */
class SubChannel1PhaseProblemBase : public ExternalProblem
{
public:
  SubChannel1PhaseProblemBase(const InputParameters & params);
  virtual ~SubChannel1PhaseProblemBase();

  virtual void externalSolve() override;
  virtual void syncSolutions(Direction direction) override;
  virtual bool converged() override;
  virtual void initialSetup() override;

protected:
  /// Computes diversion crossflow per gap for level iz
  virtual void computeWij(int iz);
  /// Computes net diversion crossflow per channel for level iz
  virtual void computeSumWij(int iz);
  /// Computes mass flow per channel for level iz
  virtual void computeMdot(int iz);
  /// Returns friction factor
  virtual double computeFrictionFactor(double Re) = 0;
  /// Returns mass flow for a given pressure drop
  virtual double computeMassFlowForDPDZ(double dpdz, int i_ch);
  /// Computes turbulent crossflow per gap for level iz
  virtual void computeWijPrime(int iz);
  /// Computes Pressure Drop per channel for level iz
  virtual void computeDP(int iz);
  /// Computes Pressure per channel for level iz
  virtual void computeP(int iz);
  /// Computes Enthalpy per channel for level iz
  virtual void computeH(int iz);
  /// Computes Temperature per channel for level iz
  virtual void computeT(int iz);
  /// Computes Density per channel for level iz
  virtual void computeRho(int iz);
  /// Computes and populates solution vector with Boundary mass flow
  virtual void enforceUniformDPDZAtInlet();

  Eigen::MatrixXd Wij;
  Eigen::MatrixXd Wij_old;
  Eigen::MatrixXd WijPrime;
  const Real _g_grav;
  Real _one;
  Real _zero;
  /// Flag that activates or deactivates the transient parts of the equations solved
  Real _TR;
  /// Time step
  const Real & _dt;
  SubChannelMeshBase & _subchannel_mesh;
  /// Thermal diffusion coefficient used in turbulent crossflow
  const Real & _abeta;
  /// Turbulent modeling parameter used in axial momentum equation
  const Real & _CT;
  /// Flag that indicates if uniform pressure should be applied at the subchannel inlet
  const bool & _enforce_uniform_pressure;
  const SinglePhaseFluidProperties * _fp;
  SolutionHandle * mdot_soln;
  SolutionHandle * SumWij_soln;
  SolutionHandle * P_soln;
  SolutionHandle * DP_soln;
  SolutionHandle * h_soln;
  SolutionHandle * T_soln;
  SolutionHandle * rho_soln;
  SolutionHandle * S_flow_soln;
  SolutionHandle * w_perim_soln;
  SolutionHandle * q_prime_soln;

public:
  static InputParameters validParams();
};
