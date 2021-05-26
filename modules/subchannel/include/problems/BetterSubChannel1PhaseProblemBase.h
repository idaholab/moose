#pragma once
#include "ExternalProblem.h"
#include "SubChannelApp.h"
#include "BetterSubChannelMeshBase.h"
#include "SolutionHandle.h"
#include "SinglePhaseFluidProperties.h"

class BetterSubChannel1PhaseProblemBase;

/**
 * Base class for the 1-phase steady state sub channel solver.
 */
class BetterSubChannel1PhaseProblemBase : public ExternalProblem
{
public:
  BetterSubChannel1PhaseProblemBase(const InputParameters & params);
  virtual ~BetterSubChannel1PhaseProblemBase();

  virtual void externalSolve() override;
  virtual void syncSolutions(Direction direction) override;
  virtual bool converged() override;
  virtual void initialSetup() override;

protected:
  /// Returns friction factor
  virtual double computeFrictionFactor(double Re);
  /// Computes diversion crossflow per gap for block iblock
  virtual void computeWij(int iblock);
  /// Computes net diversion crossflow per channel for block iblock
  virtual void computeSumWij(int iblock);
  /// Computes mass flow per channel for block iblock
  virtual void computeMdot(int iblock);
  /// Computes turbulent crossflow per gap for block iblock
  virtual void computeWijPrime(int iblock);
  /// Computes Pressure Drop per channel for block iblock
  virtual void computeDP(int iblock);
  /// Computes Pressure per channel for block iblock
  virtual void computeP(int iblock);
  /// Computes Enthalpy per channel for block iblock
  virtual void computeh(int iblock);
  /// Computes Temperature per channel for block iblock
  virtual void computeT(int iblock);
  /// Computes Density per channel for block iblock
  virtual void computeRho(int iblock);
  /// Computes Viscosity per channel for block iblock
  virtual void computeMu(int iblock);
  /// Computes Residual per gap for block iblock
  virtual void computeResidualFunction(int iblock);

  Eigen::MatrixXd Wij;
  Eigen::MatrixXd Wij_old;
  Eigen::MatrixXd WijPrime;
  const Real _g_grav;
  unsigned int n_cells;
  unsigned int n_blocks;
  unsigned int n_gaps;
  unsigned int block_size;
  Real _one;
  /// Flag that activates or deactivates the transient parts of the equations solved
  Real _TR;
  /// Flag that activates or deactivates the calculation of density
  const bool _Density;
  /// Flag that activates or deactivates the calculation of viscosity
  const bool _Viscosity;
  /// Time step
  const Real & _dt;
  BetterSubChannelMeshBase & _subchannel_mesh;
  /// Thermal diffusion coefficient used in turbulent crossflow
  const Real & _beta;
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
  SolutionHandle * Mu_soln;
  SolutionHandle * S_flow_soln;
  SolutionHandle * w_perim_soln;
  SolutionHandle * q_prime_soln;

public:
  static InputParameters validParams();
};
