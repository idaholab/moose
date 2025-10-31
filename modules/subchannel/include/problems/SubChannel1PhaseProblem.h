//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "ExternalProblem.h"
#include "PostprocessorInterface.h"
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

/**
 * Base class for the 1-phase steady-state/transient subchannel solver.
 */
class SubChannel1PhaseProblem : public ExternalProblem, public PostprocessorInterface
{
public:
  SubChannel1PhaseProblem(const InputParameters & params);
  virtual ~SubChannel1PhaseProblem();

  virtual void externalSolve() override;
  virtual void syncSolutions(Direction direction) override;
  virtual bool solverSystemConverged(const unsigned int) override;
  virtual void initialSetup() override;

  /// Function that computes the added heat coming from the fuel pins, for channel i_ch and cell iz
  virtual Real computeAddedHeatPin(unsigned int i_ch, unsigned int iz) = 0;
  /// Function that computes the heat added by the duct, for channel i_ch and cell iz
  Real computeAddedHeatDuct(unsigned int i_ch, unsigned int iz);

protected:
  struct FrictionStruct
  {
    int i_ch;
    Real Re, S, w_perim;
  } _friction_args;

  /// Returns friction factor
  virtual Real computeFrictionFactor(FrictionStruct friction_args) = 0;
  /// Computes diversion crossflow per gap for block iblock
  void computeWijFromSolve(int iblock);
  /// Computes net diversion crossflow per channel for block iblock
  void computeSumWij(int iblock);
  /// Computes mass flow per channel for block iblock
  void computeMdot(int iblock);
  /// Computes turbulent crossflow per gap for block iblock
  void computeWijPrime(int iblock);
  /// Computes turbulent mixing coefficient
  virtual Real computeBeta(unsigned int i_gap, unsigned int iz, bool enthalpy) = 0;
  /// Computes Pressure Drop per channel for block iblock
  void computeDP(int iblock);
  /// Computes Pressure per channel for block iblock
  void computeP(int iblock);
  /// Computes Enthalpy per channel for block iblock
  virtual void computeh(int iblock) = 0;
  /// Computes Temperature per channel for block iblock
  void computeT(int iblock);
  /// Computes Density per channel for block iblock
  void computeRho(int iblock);
  /// Computes Viscosity per channel for block iblock
  void computeMu(int iblock);
  /// Computes Residual Matrix based on the lateral momentum conservation equation for block iblock
  void computeWijResidual(int iblock);
  /// Function that computes the width of the duct cell that the peripheral subchannel i_ch sees
  virtual Real getSubChannelPeripheralDuctWidth(unsigned int i_ch) = 0;
  /// Computes Residual Vector based on the lateral momentum conservation equation for block iblock & updates flow variables based on current crossflow solution
  libMesh::DenseVector<Real> residualFunction(int iblock, libMesh::DenseVector<Real> solution);
  /// Computes solution of nonlinear equation using snes and provided a residual in a formFunction
  PetscErrorCode petscSnesSolver(int iblock,
                                 const libMesh::DenseVector<Real> & solution,
                                 libMesh::DenseVector<Real> & root);
  /// This is the residual Vector function in a form compatible with the SNES PETC solvers
  friend PetscErrorCode formFunction(SNES snes, Vec x, Vec f, void * ctx);

  /// Computes implicit solve using PetSc
  PetscErrorCode implicitPetscSolve(int iblock);

  /// Function to initialize the solution & geometry fields
  virtual void initializeSolution() = 0;

  /// Functions that computes the interpolation scheme given the Peclet number
  PetscScalar computeInterpolationCoefficients(PetscScalar Peclet = 0.0);
  PetscScalar
  computeInterpolatedValue(PetscScalar topValue, PetscScalar botValue, PetscScalar Peclet = 0.0);

  /// inline function that is used to define the gravity direction
  Real computeGravityDir(const MooseEnum & dir) const
  {
    switch (dir)
    {
      case 0: // counter_flow
        return 1.0;
      case 1: // co_flow
        return -1.0;
      case 2: // none
        return 0.0;
      default:
        mooseError(name(), ": Invalid gravity direction: expected counter_flow, co_flow, or none");
    }
  }

  PetscErrorCode cleanUp();
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
  /// Flag that informs if there is a duct mesh or not
  const bool _duct_mesh_exist;
  /// Variable that informs whether we exited external solve with a converged solution or not
  bool _converged;
  /// Time step
  Real _dt;
  /// Outlet Pressure
  const PostprocessorValue & _P_out;
  /// Turbulent modeling parameter used in axial momentum equation
  const Real & _CT;
  /// Convergence tolerance for the pressure loop in external solve
  const Real & _P_tol;
  /// Convergence tolerance for the temperature loop in internal solve
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
  const MooseEnum _interpolation_scheme;
  /// The direction of gravity
  const MooseEnum _gravity_direction;
  const Real _dir_grav;
  /// Flag to define the usage of a implicit or explicit solution
  const bool _implicit_bool;
  /// Flag to define the usage of staggered or collocated pressure
  const bool _staggered_pressure_bool;
  /// Segregated solve
  const bool _segregated_bool;
  /// Thermal monolithic bool
  const bool _monolithic_thermal_bool;
  /// Boolean to printout information related to subchannel solve
  const bool _verbose_subchannel;
  /// Flag that activates the effect of deformation (pin/duct) based on the auxvalues for displacement, Dpin
  const bool _deformation;

  /// Solutions handles and link to TH tables properties
  const SinglePhaseFluidProperties * _fp;
  std::unique_ptr<SolutionHandle> _mdot_soln;
  std::unique_ptr<SolutionHandle> _SumWij_soln;
  std::unique_ptr<SolutionHandle> _P_soln;
  std::unique_ptr<SolutionHandle> _DP_soln;
  std::unique_ptr<SolutionHandle> _h_soln;
  std::unique_ptr<SolutionHandle> _T_soln;
  std::unique_ptr<SolutionHandle> _Tpin_soln;
  std::unique_ptr<SolutionHandle> _Dpin_soln;
  std::unique_ptr<SolutionHandle> _rho_soln;
  std::unique_ptr<SolutionHandle> _mu_soln;
  std::unique_ptr<SolutionHandle> _S_flow_soln;
  std::unique_ptr<SolutionHandle> _w_perim_soln;
  std::unique_ptr<SolutionHandle> _q_prime_soln;
  std::unique_ptr<SolutionHandle> _duct_heat_flux_soln; // Only used for ducted assemblies
  std::unique_ptr<SolutionHandle> _Tduct_soln;          // Only used for ducted assemblies
  std::unique_ptr<SolutionHandle> _displacement_soln;

  /// Petsc Functions
  inline PetscErrorCode createPetscVector(Vec & v, PetscInt n)
  {
    PetscFunctionBegin;
    LibmeshPetscCall(VecCreate(PETSC_COMM_SELF, &v));
    LibmeshPetscCall(PetscObjectSetName((PetscObject)v, "Solution"));
    LibmeshPetscCall(VecSetSizes(v, PETSC_DECIDE, n));
    LibmeshPetscCall(VecSetFromOptions(v));
    LibmeshPetscCall(VecZeroEntries(v));
    PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
  }

  inline PetscErrorCode createPetscMatrix(Mat & M, PetscInt n, PetscInt m)
  {
    PetscFunctionBegin;
    LibmeshPetscCall(MatCreate(PETSC_COMM_SELF, &M));
    LibmeshPetscCall(MatSetSizes(M, PETSC_DECIDE, PETSC_DECIDE, n, m));
    LibmeshPetscCall(MatSetFromOptions(M));
    LibmeshPetscCall(MatSetUp(M));
    PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
  }

  template <class T>
  PetscErrorCode populateVectorFromDense(Vec & x,
                                         const T & solution,
                                         const unsigned int first_axial_level,
                                         const unsigned int last_axial_level,
                                         const unsigned int cross_dimension);
  template <class T>
  PetscErrorCode populateDenseFromVector(const Vec & x,
                                         T & solution,
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

  //// Matrices and vectors to be used in implicit assembly
  /// Mass conservation
  /// Mass conservation - sum of cross fluxes
  Mat _mc_sumWij_mat;
  Vec _Wij_vec;
  Vec _prod;
  Vec _prodp;
  /// Mass conservation - axial convection
  Mat _mc_axial_convection_mat;
  Vec _mc_axial_convection_rhs;
  /// Mass conservation - density time derivative
  /// No implicit matrix

  /// Axial momentum
  /// Axial momentum conservation - compute turbulent cross fluxes
  Mat _amc_turbulent_cross_flows_mat;
  Vec _amc_turbulent_cross_flows_rhs;
  /// Axial momentum conservation - time derivative
  Mat _amc_time_derivative_mat;
  Vec _amc_time_derivative_rhs;
  /// Axial momentum conservation - advective (Eulerian) derivative
  Mat _amc_advective_derivative_mat;
  Vec _amc_advective_derivative_rhs;
  /// Axial momentum conservation - cross flux derivative
  Mat _amc_cross_derivative_mat;
  Vec _amc_cross_derivative_rhs;
  /// Axial momentum conservation - friction force
  Mat _amc_friction_force_mat;
  Vec _amc_friction_force_rhs;
  /// Axial momentum conservation - buoyancy force
  /// No implicit matrix
  Vec _amc_gravity_rhs;
  /// Axial momentum conservation - pressure force
  Mat _amc_pressure_force_mat;
  Vec _amc_pressure_force_rhs;
  /// Axial momentum system matrix
  Mat _amc_sys_mdot_mat;
  Vec _amc_sys_mdot_rhs;

  /// Cross momentum
  /// Cross momentum conservation - time derivative
  Mat _cmc_time_derivative_mat;
  Vec _cmc_time_derivative_rhs;
  /// Cross momentum conservation - advective (Eulerian) derivative
  Mat _cmc_advective_derivative_mat;
  Vec _cmc_advective_derivative_rhs;
  /// Cross momentum conservation - friction force
  Mat _cmc_friction_force_mat;
  Vec _cmc_friction_force_rhs;
  /// Cross momentum conservation - pressure force
  Mat _cmc_pressure_force_mat;
  Vec _cmc_pressure_force_rhs;
  /// Lateral momentum system matrix
  Mat _cmc_sys_Wij_mat;
  Vec _cmc_sys_Wij_rhs;

  /// Enthalpy
  /// Enthalpy conservation - time derivative
  Mat _hc_time_derivative_mat;
  Vec _hc_time_derivative_rhs;
  /// Enthalpy conservation - advective (Eulerian) derivative;
  Mat _hc_advective_derivative_mat;
  Vec _hc_advective_derivative_rhs;
  /// Enthalpy conservation - cross flux derivative
  Mat _hc_cross_derivative_mat;
  Vec _hc_cross_derivative_rhs;
  /// Enthalpy conservation - source and sink
  Vec _hc_added_heat_rhs;
  /// System matrices
  Mat _hc_sys_h_mat;
  Vec _hc_sys_h_rhs;

  /// Added resistances for monolithic convergence
  PetscScalar _added_K = 0.0;
  PetscScalar _added_K_old = 1000.0;
  PetscScalar _max_sumWij;
  PetscScalar _max_sumWij_new;
  PetscScalar _correction_factor = 1.0;

public:
  static InputParameters validParams();
};

template <class T>
PetscErrorCode
SubChannel1PhaseProblem::populateDenseFromVector(const Vec & x,
                                                 T & loc_solution,
                                                 const unsigned int first_axial_level,
                                                 const unsigned int last_axial_level,
                                                 const unsigned int cross_dimension)
{
  PetscScalar * xx;

  PetscFunctionBegin;
  LibmeshPetscCall(VecGetArray(x, &xx));
  for (unsigned int iz = first_axial_level; iz < last_axial_level + 1; iz++)
  {
    unsigned int iz_ind = iz - first_axial_level;
    for (unsigned int i_l = 0; i_l < cross_dimension; i_l++)
    {
      loc_solution(i_l, iz) = xx[iz_ind * cross_dimension + i_l];
    }
  }
  LibmeshPetscCall(VecRestoreArray(x, &xx));

  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}

template <class T>
PetscErrorCode
SubChannel1PhaseProblem::populateVectorFromHandle(Vec & x,
                                                  const T & loc_solution,
                                                  const unsigned int first_axial_level,
                                                  const unsigned int last_axial_level,
                                                  const unsigned int cross_dimension)
{
  PetscScalar * xx;

  PetscFunctionBegin;
  LibmeshPetscCall(VecGetArray(x, &xx));
  for (unsigned int iz = first_axial_level; iz < last_axial_level + 1; iz++)
  {
    unsigned int iz_ind = iz - first_axial_level;
    for (unsigned int i_l = 0; i_l < cross_dimension; i_l++)
    {
      auto * loc_node = _subchannel_mesh.getChannelNode(i_l, iz);
      xx[iz_ind * cross_dimension + i_l] = loc_solution(loc_node);
    }
  }
  LibmeshPetscCall(VecRestoreArray(x, &xx));

  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}

template <class T>
PetscErrorCode
SubChannel1PhaseProblem::populateVectorFromDense(Vec & x,
                                                 const T & loc_solution,
                                                 const unsigned int first_axial_level,
                                                 const unsigned int last_axial_level,
                                                 const unsigned int cross_dimension)
{
  PetscScalar * xx;
  PetscFunctionBegin;
  LibmeshPetscCall(VecGetArray(x, &xx));
  for (unsigned int iz = first_axial_level; iz < last_axial_level + 1; iz++)
  {
    unsigned int iz_ind = iz - first_axial_level;
    for (unsigned int i_l = 0; i_l < cross_dimension; i_l++)
    {
      xx[iz_ind * cross_dimension + i_l] = loc_solution(i_l, iz);
    }
  }
  LibmeshPetscCall(VecRestoreArray(x, &xx));
  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}

template <class T>
PetscErrorCode
SubChannel1PhaseProblem::populateSolutionChan(const Vec & x,
                                              T & loc_solution,
                                              const unsigned int first_axial_level,
                                              const unsigned int last_axial_level,
                                              const unsigned int cross_dimension)
{
  PetscScalar * xx;
  PetscFunctionBegin;
  LibmeshPetscCall(VecGetArray(x, &xx));
  Node * loc_node;
  for (unsigned int iz = first_axial_level; iz < last_axial_level + 1; iz++)
  {
    unsigned int iz_ind = iz - first_axial_level;
    for (unsigned int i_l = 0; i_l < cross_dimension; i_l++)
    {
      loc_node = _subchannel_mesh.getChannelNode(i_l, iz);
      loc_solution.set(loc_node, xx[iz_ind * cross_dimension + i_l]);
    }
  }
  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}
