#pragma once
#include "ExternalProblem.h"
#include "SubChannelApp.h"
#include "BetterSubChannelMeshBase.h"
#include "SolutionHandle.h"
#include "SinglePhaseFluidProperties.h"

class BetterSubChannel1PhaseProblem;

/**
 * Base class for the 1-phase steady state sub channel solver.
 */
class BetterSubChannel1PhaseProblem : public ExternalProblem
{
public:
  BetterSubChannel1PhaseProblem(const InputParameters & params);
  virtual ~BetterSubChannel1PhaseProblem();

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
  virtual Eigen::VectorXd residualFunction(int iblock, Eigen::VectorXd solution);
  /// Computes solution of nonlinear equation using snes
  virtual PetscErrorCode
  petscSnesSolver(int iblock, const Eigen::VectorXd & solution, Eigen::VectorXd & root);
  friend PetscErrorCode formFunction(SNES snes, Vec x, Vec f, void * ctx);

  Eigen::MatrixXd _Wij;
  Eigen::MatrixXd _Wij_old;
  Eigen::MatrixXd _WijPrime;
  const Real _g_grav;
  unsigned int _n_cells;
  unsigned int _n_blocks;
  unsigned int _n_gaps;
  unsigned int _n_channels;
  unsigned int _block_size;
  Real _one;
  /// Flag that activates or deactivates the transient parts of the equations solved by multiplication
  Real _TR;
  /// Flag that activates or deactivates the calculation of density
  const bool _compute_density;
  /// Flag that activates or deactivates the calculation of viscosity
  const bool _compute_viscosity;
  /// Flag that informs where we solve the Enthalpy/Temperature equations or not
  const bool _compute_power;
  /// Time step
  const Real & _dt;
  /// Outlet Pressure
  const Real & _P_out;
  BetterSubChannelMeshBase & _subchannel_mesh;
  /// Thermal diffusion coefficient used in turbulent crossflow
  const Real & _beta;
  /// Turbulent modeling parameter used in axial momentum equation
  const Real & _CT;
  /// Flag that indicates if uniform pressure should be applied at the subchannel inlet
  const bool & _enforce_uniform_pressure;
  const SinglePhaseFluidProperties * _fp;
  SolutionHandle * _mdot_soln;
  SolutionHandle * _SumWij_soln;
  SolutionHandle * _P_soln;
  SolutionHandle * _DP_soln;
  SolutionHandle * _h_soln;
  SolutionHandle * _T_soln;
  SolutionHandle * _rho_soln;
  SolutionHandle * _mu_soln;
  SolutionHandle * _S_flow_soln;
  SolutionHandle * _w_perim_soln;
  SolutionHandle * _q_prime_soln;

public:
  static InputParameters validParams();
};
