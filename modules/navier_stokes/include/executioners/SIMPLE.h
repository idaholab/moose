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
 * Executioner set up to solve a thermal-hydraulics problem using the SIMPLE algorithm.
 * It utilizes segregated linearized systems which are solved using a fixed-point iteration.
 */
class SIMPLE : public Executioner
{
public:
  static InputParameters validParams();

  SIMPLE(const InputParameters & parameters);

  void init() override;
  void execute() override;
  bool lastSolveConverged() const override { return _last_solve_converged; }

  const INSFVRhieChowInterpolatorSegregated & getRCUserObject() { return *_rc_uo; }

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

  /// Solve a momentum predictor step with a fixed pressure field
  /// @return A vector for the normalized residual norms of the momentum equations.
  ///         The length of the vector equals the dimensionality of the domain.
  std::vector<Real> solveMomentumPredictor();

  /// Solve a pressure corrector step.
  /// @return The normalized residual norm of the pressure equation.
  Real solvePressureCorrector();

  /// Solve an equation which contains an advection term that depends
  /// on the solution of the segregated Navier-Stokes equations.
  /// @param system_num The number of the system which is solved
  /// @param system Reference to the system which is solved
  /// @param relaxation_factor The relaxation factor for matrix relaxation
  /// @param solver_config The solver configuration object for the linear solve
  /// @param abs_tol The scaled absolute tolerance for the linear solve
  /// @return The normalized residual norm of the equation.
  Real solveAdvectedSystem(const unsigned int system_num,
                           NonlinearSystemBase & system,
                           const Real relaxation_factor,
                           SolverConfiguration & solver_config,
                           const Real abs_tol);

  /// Solve the solid energy conservation equation.
  /// @return The normalized residual norm of the solid equation.
  Real solveSolidEnergySystem();

  /**
   * Relax the update on a solution field using the following approach:
   * $u = u_{old}+\lambda (u - u_{old})$
   *
   * @param system_in The system whose solution shall be relaxed
   * @param relaxation_factor The lambda parameter in the expression above
   */
  void relaxSolutionUpdate(NonlinearSystemBase & system_in, Real relaxation_factor);

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
   * @param ns_residuals The current residuals for the systems.
   * @param abs_tolerances The corresponding absolute tolerances.
   */
  bool converged(const std::vector<Real> & residuals, const std::vector<Real> & abs_tolerances);

  FEProblemBase & _problem;
  Real _system_time;
  int & _time_step;
  Real & _time;

  /// Boolean for easy check if a fluid energy system shall be solved or not
  const bool _has_energy_system;

  /// Boolean for easy check if a solid energy system shall be solved or not
  const bool _has_solid_energy_system;

  /// Boolean for easy check if a passive scalar systems shall be solved or not
  const bool _has_passive_scalar_systems;

  /// The names of the momentum systems.
  const std::vector<NonlinearSystemName> _momentum_system_names;

  /// The number(s) of the system(s) corresponding to the momentum equation(s)
  std::vector<unsigned int> _momentum_system_numbers;

  /// The number of the system corresponding to the pressure equation
  const unsigned int _pressure_sys_number;

  /// The number of the system corresponding to the energy equation
  const unsigned int _energy_sys_number;

  /// The number of the system corresponding to the solid energy equation
  const unsigned int _solid_energy_sys_number;

  /// The number(s) of the system(s) corresponding to the passive scalar equation(s)
  std::vector<unsigned int> _passive_scalar_system_numbers;

  /// Pointer(s) to the system(s) corresponding to the momentum equation(s)
  std::vector<NonlinearSystemBase *> _momentum_systems;

  /// Reference to the nonlinear system corresponding to the pressure equation
  NonlinearSystemBase & _pressure_system;

  /// Pointer to the nonlinear system corresponding to the fluid energy equation
  NonlinearSystemBase * _energy_system;

  /// Pointer to the nonlinear system corresponding to the solid energy equation
  NonlinearSystemBase * _solid_energy_system;

  /// Pointer(s) to the system(s) corresponding to the passive scalar equation(s)
  std::vector<NonlinearSystemBase *> _passive_scalar_systems;

  /// Pointer to the segregated RhieChow interpolation object
  INSFVRhieChowInterpolatorSegregated * _rc_uo;

private:
  void checkDependentParameterError(const std::string & main_parameter,
                                    const std::vector<std::string> & dependent_parameters,
                                    const bool should_be_defined);

  /// Check for wrong execute on flags in the multiapps
  bool hasMultiAppError(const ExecFlagEnum & flags);

  /// Check for wrong execute on flags in the transfers
  bool hasTransferError(const ExecFlagEnum & flags);

  /**
   * Check if the system contains a time kernel or not. This is a steady-state executioner so we
   * wanna make sure the user doesn't have kernels that add time-derivatives.
   * @param system Reference to the system which holds the kernels. This is not const because
   * the constainsTimeKernel() routine is not const.
   */
  void checkIntegrity(NonlinearSystemBase & system);

  bool _last_solve_converged;

  /// The name of the vector tag which corresponds to the pressure gradient terms in the
  /// momentum equation
  const TagName _pressure_tag_name;

  /// The ID of the tag which corresponds to the pressure gradient terms in the
  /// momentum equation
  const TagID _pressure_tag_id;

  /// The user-defined relaxation parameter for the momentum equation
  const Real _momentum_equation_relaxation;

  /// The user-defined relaxation parameter for the pressure variable
  const Real _pressure_variable_relaxation;

  /// The user-defined relaxation parameter for the energy equation
  const Real _energy_equation_relaxation;

  /// The user-defined relaxation parameter(s) for the passive scalar equation(s)
  const std::vector<Real> _passive_scalar_equation_relaxation;

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

  /// The maximum number of momentum-pressure iterations
  const unsigned int _num_iterations;

  /// Debug parameter which allows printing the coupling and solution vectors/matrices
  const bool _print_fields;

  /// Options which hold the petsc settings for the momentum equation
  Moose::PetscSupport::PetscOptions _momentum_petsc_options;

  /// Options which hold the petsc settings for the pressure equation
  Moose::PetscSupport::PetscOptions _pressure_petsc_options;

  /// Options which hold the petsc settings for the energy equation
  Moose::PetscSupport::PetscOptions _energy_petsc_options;

  /// Options which hold the petsc settings for the energy equation
  Moose::PetscSupport::PetscOptions _solid_energy_petsc_options;

  /// Options which hold the petsc settings for the passive scalar equation(s)
  Moose::PetscSupport::PetscOptions _passive_scalar_petsc_options;

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

  /// If the pressure needs to be pinned
  const bool _pin_pressure;

  /// The value we want to enforce for pressure
  const bool _pressure_pin_value;

  /// The dof ID where the pressure needs to be pinned
  dof_id_type _pressure_pin_dof;
};
