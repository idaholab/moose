#pragma once
#include "ExternalProblem.h"
#include "SubChannelApp.h"
#include "QuadSubChannelMesh.h"
#include "SolutionHandle.h"
#include "SinglePhaseFluidProperties.h"
#include <petscdm.h>
#include <petscdmda.h>
#include <petscksp.h>
#include <petscsys.h>
#include <petscvec.h>
#include <petscsnes.h>

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

  /// Standard return structure for reusing in implicit/explicit formulations
  struct StructPetscMatVec{ Mat A; Vec x;};

  /// Returns friction factor
  virtual double computeFrictionFactor(double Re) = 0;
  /// Computes diversion crossflow per gap for block iblock
  virtual void computeWijFromSolve(int iblock);
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
  /// Computes cross fluxes for block iblock
  virtual void computeWij(int iblock);
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

  /// Computes implicit solve using PetSc
  virtual PetscErrorCode implicitPetscSolve(int iblock);
  /// Function to initialize the solution fields
  virtual void initializeSolution();

  /// Functions that computes the interpolation scheme given the Peclet number
  virtual PetscScalar computeInterpolationCoefficients(const std::string interp_type,
                                                       PetscScalar Peclet);
  virtual PetscScalar computeInterpolatedValue(PetscScalar topValue,
                                               PetscScalar botValue,
                                               const std::string interp_type,
                                               PetscScalar Peclet);

  SubChannelMesh & _subchannel_mesh;
  /// number of axial blocks
  unsigned int _n_blocks;
  libMesh::DenseMatrix<Real> & _Wij;
  libMesh::DenseMatrix<Real> _Wij_old;
  libMesh::DenseMatrix<Real> _WijPrime;
  libMesh::DenseMatrix<Real> _Wij_residual_matrix;
  const Real _g_grav;
  const Real & _kij;
  unsigned int _n_cells;
  unsigned int _n_gaps;
  unsigned int _n_pins;
  unsigned int _n_channels;
  unsigned int _block_size;
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
  /// The interpolation method used in constructing the systems
  const std::string _interpolation_scheme;
  /// Flag to define the usage of a implicit or explicit solution
  const bool _implicit_bool;
  /// Flag to define the usage of staggered or collocated pressure
  const bool _staggered_pressure_bool;
  /// Segregated solve
  const bool _segregated_bool;
  /// Thermal monolithic bool
  const bool _monolithic_thermal_bool;

  /// Solutions handles and link to TH tables properties
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

  /// Petsc Functions
  virtual PetscErrorCode createPetscVector(Vec & v, PetscInt n);
  virtual PetscErrorCode createPetscMatrix(Mat & M, PetscInt n, PetscInt m);
  template <class T>
  PetscErrorCode populateVectorFromDense(Vec & x,
                                         const T & solution,
                                         const unsigned int first_axial_level,
                                         const unsigned int last_axial_level,
                                         const unsigned int cross_dimension);
  template <class T>
  PetscErrorCode populateVectorFromHandle(Vec & x,
                                          const T & solution,
                                          const unsigned int first_axial_level,
                                          const unsigned int last_axial_level,
                                          const unsigned int cross_dimension);
  template <class T>
  PetscErrorCode populateSolutionChan(const Vec & x,
                                      T & solution,
                                      const unsigned int first_axial_level,
                                      const unsigned int last_axial_level,
                                      const unsigned int cross_dimension);

  template <class T>
  PetscErrorCode populateSolutionGap(const Vec & x,
                                     T & solution,
                                     const unsigned int first_axial_level,
                                     const unsigned int last_axial_level,
                                     const unsigned int cross_dimension);

  //// Matrices and vectors to be used in implicit assembly

  /// Mass conservation
  // Mass conservation - sum of cross fluxes
  Mat mc_sumWij_mat;
  Vec Wij_vec;
  Vec prod;
  Vec prodp;
  // Mass conservation - axial convection
  Mat mc_axial_convection_mat;
  Vec mc_axial_convection_rhs;
  // Mass conservation - density time derivative
  // No implicit matrix

  /// Axial momentum
  // Axial momentum conservation - compute turbulent cross fluxes
  Mat amc_turbulent_cross_flows_mat;
  Vec amc_turbulent_cross_flows_rhs;
  // Axial momentum conservation - time derivative
  Mat amc_time_derivative_mat;
  Vec amc_time_derivative_rhs;
  // Axial momentum conservation - advective (Eulerian) derivative
  Mat amc_advective_derivative_mat;
  Vec amc_advective_derivative_rhs;
  // Axial momentum conservation - cross flux derivative
  Mat amc_cross_derivative_mat;
  Vec amc_cross_derivative_rhs;
  // Axial momentum conservation - friction force
  Mat amc_friction_force_mat;
  Vec amc_friction_force_rhs;
  // Axial momentum conservation - buoyancy force
  // No implicit matrix
  Vec amc_gravity_rhs;
  // Axial momentum conservation - pressure force
  Mat amc_pressure_force_mat;
  Vec amc_pressure_force_rhs;
  // Axial momentum system matrix
  Mat amc_sys_mdot_mat;
  Vec amc_sys_mdot_rhs;

  /// Cross momentum
  // Cross momentum conservation - time derivative
  Mat cmc_time_derivative_mat;
  Vec cmc_time_derivative_rhs;
  // Cross momentum conservation - advective (Eulerian) derivative
  Mat cmc_advective_derivative_mat;
  Vec cmc_advective_derivative_rhs;
  // Cross momentum conservation - friction force
  Mat cmc_friction_force_mat;
  Vec cmc_friction_force_rhs;
  // Cross momentum conservation - pressure force
  Mat cmc_pressure_force_mat;
  Vec cmc_pressure_force_rhs;
  // Lateral momentum system matrix
  Mat cmc_sys_Wij_mat;
  Vec cmc_sys_Wij_rhs;
  Vec cmc_Wij_channel_dummy;

  /// Enthalpy
  // Enthalpy conservation - time derivative
  Mat hc_time_derivative_mat;
  Vec hc_time_derivative_rhs;
  // Enthalpy conservation - advective (Eulerian) derivative;
  Mat hc_advective_derivative_mat;
  Vec hc_advective_derivative_rhs;
  // Enthalpy conservation - cross flux derivative
  Mat hc_cross_derivative_mat;
  Vec hc_cross_derivative_rhs;
  // Enthalpy conservation - source and sink
  Vec hc_added_heat_rhs;
  // System matrices
  Mat hc_sys_h_mat;
  Vec hc_sys_h_rhs;
  // No implicit matrix
  PetscInt _global_counter = 0;

  /// Added resistances for monolithic convergence
  PetscScalar _added_K = 0.0;
  PetscScalar _added_K_old = 1000.0;
  PetscScalar max_sumWij;
  PetscScalar max_sumWij_new;
  PetscScalar correction_factor = 1.0;



public:
  static InputParameters validParams();
};
