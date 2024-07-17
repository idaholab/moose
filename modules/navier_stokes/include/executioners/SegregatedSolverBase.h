//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Executioner.h"
#include "INSFVRhieChowInterpolatorSegregated.h"
#include "PetscSupport.h"
#include "SolverParams.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/equation_systems.h"
#include "libmesh/solver_configuration.h"

// Forward declarations
class InputParameters;
class FEProblemBase;

/**
 * Solver configuration class used with the linear solvers in a SIMPLE solver.
 */
class SIMPLESolverConfiguration : public libMesh::SolverConfiguration
{
  /**
   * Override this to make sure the PETSc options are not overwritten in the linear solver
   */
  virtual void configure_solver() override {}
};

/**
 * Base class for the executioners relying on segregated solution approaches.
 */
class SegregatedSolverBase : public Executioner
{
public:
  static InputParameters validParams();

  SegregatedSolverBase(const InputParameters & parameters);

  virtual void init() override;
  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

  /**
   * Compute a normalization factor which is applied to the linear residual to determine
   * convergence. This function is based on the description provided here:
   *  // @article{greenshields2022notes,
   * title={Notes on computational fluid dynamics: General principles},
   * author={Greenshields, Christopher J and Weller, Henry G},
   * journal={(No Title)},
   * year={2022}
   * }
   * @param solution The solution vector
   * @param mat The system matrix
   * @param rhs The system right hand side
   */
  Real computeNormalizationFactor(const NumericVector<Number> & solution,
                                  const SparseMatrix<Number> & mat,
                                  const NumericVector<Number> & rhs);

protected:
  /**
   * Relax the matrix to ensure diagonal dominance, we hold onto the difference in diagonals
   * for later use in relaxing the right hand side. For the details of this relaxation process, see
   *
   * Juretic, Franjo. Error analysis in finite volume CFD. Diss.
   * Imperial College London (University of London), 2005.
   *
   * @param matrix_in The matrix that needs to be relaxed
   * @param relaxation_parameter The scale which described how much the matrix is relaxed
   * @param diff_diagonal A vector holding the $A_{diag}-A_{diag, relaxed}$ entries for further
   *                      use in the relaxation of the right hand side
   */
  void relaxMatrix(SparseMatrix<Number> & matrix_in,
                   const Real relaxation_parameter,
                   NumericVector<Number> & diff_diagonal);

  /**
   * Relax the right hand side of an equation, this needs to be called once and the system matrix
   * has been relaxed and the field describing the difference in the diagonals of the system matrix
   * is already available. The relaxation process needs modification to both the system matrix and
   * the right hand side. For more information see:
   *
   * Juretic, Franjo. Error analysis in finite volume CFD. Diss.
   * Imperial College London (University of London), 2005.
   *
   * @param rhs_in The unrelaxed right hand side that needs to be relaxed
   * @param solution_in The solution
   * @param diff_diagonal The diagonal correction used for the corresponding system matrix
   */
  void relaxRightHandSide(NumericVector<Number> & rhs_in,
                          const NumericVector<Number> & solution_in,
                          const NumericVector<Number> & diff_diagonal);

  /**
   * Relax the update on a solution field using the following approach:
   * $u = u_{old}+\lambda (u - u_{old})$
   *
   * @param vector_new The new solution vector
   * @param vec_old The old solution vector
   * @param relaxation_factor The lambda parameter in the expression above
   */
  void relaxSolutionUpdate(NumericVector<Number> & vec_new,
                           const NumericVector<Number> & vec_old,
                           const Real relaxation_factor);

  /**
   * Limit a solution to its minimum and maximum bounds:
   * $u = min(max(u, min_limit), max_limit)$
   *
   * @param system_in The system whose solution shall be limited
   * @param min_limit = 0.0 The minimum limit for the solution
   * @param max_limit = 1e10 The maximum limit for the solution
   */
  void limitSolutionUpdate(NumericVector<Number> & solution,
                           const Real min_limit = std::numeric_limits<Real>::epsilon(),
                           const Real max_limit = 1e10);

  /**
   * Implicitly constrain the system by adding a factor*(u-u_desired) to it at a desired dof
   * value. To make sure the conditioning of the matrix does not change significantly, factor
   * is chosen to be the diagonal component of the matrix coefficients for a given dof.
   * @param mx The matrix of the system which needs to be constrained
   * @param rhs The right hand side of the system which needs to be constrained
   * @param value The desired value for the solution field at a dof
   * @param dof_id The ID of the dof which needs to be constrained
   */
  void constrainSystem(SparseMatrix<Number> & mx,
                       NumericVector<Number> & rhs,
                       const Real desired_value,
                       const dof_id_type dof_id);

  /**
   * Find the ID of the degree of freedom which corresponds to the variable and
   * a given point on the mesh
   * @param var_name The name of the variable
   * @param point The point on the mesh
   */
  dof_id_type findDoFID(const VariableName & var_name, const Point & point);

  /**
   * Determine if the iterative process on a set of equations converged or not
   * @param residuals The current (linear iteration number, residual) pairs for the systems.
   * @param abs_tolerances The corresponding absolute tolerances.
   */
  bool converged(const std::vector<std::pair<unsigned int, Real>> & residuals,
                 const std::vector<Real> & abs_tolerances);

  void checkDependentParameterError(const std::string & main_parameter,
                                    const std::vector<std::string> & dependent_parameters,
                                    const bool should_be_defined);

  /// Check for wrong execute on flags in the multiapps
  bool hasMultiAppError(const ExecFlagEnum & flags);

  /// Check for wrong execute on flags in the transfers
  bool hasTransferError(const ExecFlagEnum & flags);

  /// Reference to the MOOSE Problem which contains this executioner
  FEProblemBase & _problem;

  /// Time-related member variables. Only used to set the steady-state result
  /// at time 1.0;
  Real _system_time;
  int & _time_step;
  Real & _time;

  bool _last_solve_converged;

  /// Boolean for easy check if a fluid energy system shall be solved or not
  const bool _has_energy_system;

  /// Boolean for easy check if a solid energy system shall be solved or not
  const bool _has_solid_energy_system;

  /// Boolean for easy check if a passive scalar systems shall be solved or not
  const bool _has_passive_scalar_systems;

  /// Boolean for easy check if turbulence systems shall be solved or not
  const bool _has_turbulence_systems;

  /// The names of the momentum systems.
  const std::vector<SolverSystemName> & _momentum_system_names;

  /// The names of the passive scalar systems
  const std::vector<SolverSystemName> & _passive_scalar_system_names;

  /// The names of the turbulence scalar systems
  const std::vector<SolverSystemName> & _turbulence_system_names;

  /// The user-defined relaxation parameter for the momentum equation
  const Real _momentum_equation_relaxation;

  /// The user-defined relaxation parameter for the pressure variable
  const Real _pressure_variable_relaxation;

  /// The user-defined relaxation parameter for the energy equation
  const Real _energy_equation_relaxation;

  /// The user-defined relaxation parameter(s) for the passive scalar equation(s)
  const std::vector<Real> _passive_scalar_equation_relaxation;

  /// The user-defined relaxation parameter(s) for the turbulence equation(s)
  const std::vector<Real> _turbulence_equation_relaxation;

  /// The user-defined lower limit for turbulent quantities e.g. k, eps/omega, etc..
  std::vector<Real> _turbulence_field_min_limit;

  /// The user-defined absolute tolerance for determining the convergence in momentum
  const Real _momentum_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in pressure
  const Real _pressure_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in energy
  const Real _energy_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in solid energy
  const Real _solid_energy_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in passive scalars
  const std::vector<Real> _passive_scalar_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in turbulence equations
  const std::vector<Real> _turbulence_absolute_tolerance;

  /// The maximum number of momentum-pressure iterations
  const unsigned int _num_iterations;

  /// Debug parameter which allows printing the coupling and solution vectors/matrices
  const bool _print_fields;

  /// Options which hold the petsc settings for the momentum equation
  Moose::PetscSupport::PetscOptions _momentum_petsc_options;

  /// Options which hold the petsc settings for the pressure equation
  Moose::PetscSupport::PetscOptions _pressure_petsc_options;

  /// Options which hold the petsc settings for the fluid energy equation
  Moose::PetscSupport::PetscOptions _energy_petsc_options;

  /// Options which hold the petsc settings for the solid energy equation
  Moose::PetscSupport::PetscOptions _solid_energy_petsc_options;

  /// Options which hold the petsc settings for the passive scalar equation(s)
  Moose::PetscSupport::PetscOptions _passive_scalar_petsc_options;

  /// Options which hold the petsc settings for the turbulence equation(s)
  Moose::PetscSupport::PetscOptions _turbulence_petsc_options;

  /// Options for the linear solver of the momentum equation
  SIMPLESolverConfiguration _momentum_linear_control;

  /// Absolute linear tolerance for the momentum equation(s). We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _momentum_l_abs_tol;

  /// Options for the linear solver of the pressure equation
  SIMPLESolverConfiguration _pressure_linear_control;

  /// Absolute linear tolerance for the pressure equation. We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _pressure_l_abs_tol;

  /// Options for the linear solver of the energy equation
  SIMPLESolverConfiguration _energy_linear_control;

  /// Absolute linear tolerance for the energy equations. We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _energy_l_abs_tol;

  /// Options for the linear solver of the solid energy equation
  SIMPLESolverConfiguration _solid_energy_linear_control;

  /// Absolute linear tolerance for the solid energy equations. We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _solid_energy_l_abs_tol;

  /// Options for the linear solver of the passive scalar equation(s)
  SIMPLESolverConfiguration _passive_scalar_linear_control;

  /// Absolute linear tolerance for the passive scalar equation(s). We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _passive_scalar_l_abs_tol;

  /// Options for the linear solver of the turbulence equation(s)
  SIMPLESolverConfiguration _turbulence_linear_control;

  /// Absolute linear tolerance for the turbulence equation(s). We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _turbulence_l_abs_tol;

  /// If the pressure needs to be pinned
  const bool _pin_pressure;

  /// The value we want to enforce for pressure
  const Real _pressure_pin_value;

  /// The dof ID where the pressure needs to be pinned
  dof_id_type _pressure_pin_dof;
};
