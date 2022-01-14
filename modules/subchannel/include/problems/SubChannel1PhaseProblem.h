#pragma once
#include "ExternalProblem.h"
#include "SubChannelApp.h"
#include "QuadSubChannelMesh.h"
#include "SolutionHandle.h"
#include "SinglePhaseFluidProperties.h"

class SubChannel1PhaseProblem;

/**
 * Base class for the 1-phase steady-state/transient sub channel solver.
 */
class SubChannel1PhaseProblem : public ExternalProblem
{
public:
  SubChannel1PhaseProblem(const InputParameters & params);
  virtual ~SubChannel1PhaseProblem();

  virtual void externalSolve() override;
  virtual void syncSolutions(Direction direction) override;
  virtual bool converged() override;
  virtual void initialSetup() override;

protected:
  /// Returns friction factor
  virtual double computeFrictionFactor(double Re) = 0;
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
  /// Computes added heat for channel i_ch and cell iz
  virtual Real computeAddedHeat(unsigned int i_ch, unsigned int iz);
  /// Computes Residual per gap for block iblock
  virtual libMesh::DenseVector<Real> residualFunction(int iblock,
                                                      libMesh::DenseVector<Real> solution);
  /// Computes solution of nonlinear equation using snes
  virtual PetscErrorCode petscSnesSolver(int iblock,
                                         const libMesh::DenseVector<Real> & solution,
                                         libMesh::DenseVector<Real> & root);
  friend PetscErrorCode formFunction(SNES snes, Vec x, Vec f, void * ctx);

  SubChannelMesh & _subchannel_mesh;
  libMesh::DenseMatrix<Real> & _Wij;
  libMesh::DenseMatrix<Real> _Wij_old;
  libMesh::DenseMatrix<Real> _WijPrime;
  const Real _g_grav;
  const Real & _kij;
  unsigned int _n_cells;
  unsigned int _n_blocks;
  unsigned int _n_gaps;
  unsigned int _n_pins;
  unsigned int _n_channels;
  unsigned int _block_size;
  unsigned int _nx;
  unsigned int _ny;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  Real _one;
  /// Flag that activates or deactivates the transient parts of the equations we solve by multiplication
  Real _TR;
  /// Flag that activates or deactivates the calculation of density
  const bool _compute_density;
  /// Flag that activates or deactivates the calculation of viscosity
  const bool _compute_viscosity;
  /// Flag that informs if we need to solve the Enthalpy/Temperature equations or not
  const bool _compute_power;
  /// Flag that informs if there is a pin mesh or not
  const bool _pin_mesh_exist;
  /// Variable that informs whether we exited external solve with a converged solution or not
  bool _converged;
  /// Time step
  const Real & _dt;
  /// Outlet Pressure
  const Real & _P_out;
  /// Thermal diffusion coefficient used in turbulent crossflow
  const Real & _beta;
  /// Turbulent modeling parameter used in axial momentum equation
  const Real & _CT;
  /// Convergence tolerance for the pressure loop in external solve
  const Real & _P_tol;
  /// Convergence tolerance for the temperature loop in external solve
  const Real & _T_tol;
  /// Maximum iterations for the inner temperature loop
  const int & _T_maxit;
  /// The relative convergence tolerance, (relative decrease) for the ksp linear solver
  const PetscReal & _rtol;
  /// The absolute convergence tolerance for the ksp linear solver
  const PetscReal & _atol;
  /// The divergence tolerance for the ksp linear solver
  const PetscReal & _dtol;
  /// The maximum number of iterations to use for the ksp linear solver
  const PetscInt & _maxit;
  const SinglePhaseFluidProperties * _fp;
  SolutionHandle * _mdot_soln;
  SolutionHandle * _SumWij_soln;
  SolutionHandle * _P_soln;
  SolutionHandle * _DP_soln;
  SolutionHandle * _h_soln;
  SolutionHandle * _T_soln;
  SolutionHandle * _Tpin_soln;
  SolutionHandle * _rho_soln;
  SolutionHandle * _mu_soln;
  SolutionHandle * _S_flow_soln;
  SolutionHandle * _w_perim_soln;
  SolutionHandle * _q_prime_soln;

public:
  static InputParameters validParams();
};
