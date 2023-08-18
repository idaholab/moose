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

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/equation_systems.h"

// Forward declarations
class InputParameters;
class FEProblemBase;

/**
 * Something informative will come here eventually
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
   * https://www.openfoam.com/documentation/guides/latest/doc/guide-solvers-residuals.html
   * (Accessed 06/01/2023)
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
   * @param rhs_in The unrelaxed right hand side
   * @param solution_in The solution
   * @param diff_diagonal The diagonal correction used for the corresponding system matrix
   */
  void relaxRightHandSide(NumericVector<Number> & rhs_in,
                          const NumericVector<Number> & solution_in,
                          const NumericVector<Number> & diff_diagonal);

  /// Solve a momentum predictor step with a fixed pressure field
  /// @return A vector for the normalized residual norms of the momentum equations.
  ///         For monolithic cases the length of the vector is 1, while for split cases
  ///         the length equals the dimensionality of the domain.
  std::vector<Real> solveMomentumPredictor();

  /// Solve a pressure corrector step.
  /// @return The normalized residual norm of the pressure equation.
  Real solvePressureCorrector();

  /**
   * Relax the update on a solution field using the following approach:
   * $u = u_{old}+\lambda (u - u_{old})$
   *
   * @param system_in The system whose solution shall be relaxed
   * @param relaxation_factor The lambda parameter in the expression above
   */
  void relaxSolutionUpdate(NonlinearSystemBase & system_in, Real relaxation_factor);

  /**
   * Determine if the iterative process converged or not
   * @param momentum_residual The normalized residuals for the momentum equation. This can either
   *                          be the residual of the monolithic momentum equation or a vector of
   *                          residuals for the direction-wise equations.
   * @param pressure_residual The normalized residual of the pressure equation.
   */
  bool converged(const std::vector<Real> & momentum_residuals, const Real pressure_residual);

  FEProblemBase & _problem;
  Real _system_time;
  int & _time_step;
  Real & _time;

  /// Boolean for easy check if an energy system shall be solved or not
  const bool _has_energy_system;

  /// Boolean for easy check if an passive scalar systems shall be solved or not
  const bool _has_passive_scalar_systems;

  /// The names of the momentum systems. If only one provided we assume that the
  /// simulation is monolithic in terms of the momentum components.
  const std::vector<std::string> _momentum_system_names;

  /// The number(s) of the system(s) corresponding to the momentum equation(s)
  std::vector<unsigned int> _momentum_system_numbers;

  /// The number of the system corresponding to the pressure equation
  const unsigned int _pressure_sys_number;

  /// The number of the system corresponding to the energy equation
  const unsigned int _energy_sys_number;

  /// The number(s) of the system(s) corresponding to the passive scalar equation(s)
  std::vector<unsigned int> _passive_scalar_system_numbers;

  /// Pointer(s) to the system(s) corresponding to the momentum equation(s)
  std::vector<NonlinearSystemBase *> _momentum_systems;

  /// Reference to the nonlinear system corresponding to the pressure equation
  NonlinearSystemBase & _pressure_system;

  /// Reference to the nonlinear system corresponding to the energy equation
  NonlinearSystemBase * _energy_system;

  /// Pointer(s) to the system(s) corresponding to the passive scalar equation(s)
  std::vector<NonlinearSystemBase *> _passive_scalar_systems;

  /// Pointer to the segregated RhieChow interpolation object
  INSFVRhieChowInterpolatorSegregated * _rc_uo;

private:
  bool _last_solve_converged;

  /// The name of the vector tag which corresponds to the pressure gradient terms in the
  /// momentum equation
  const std::string _pressure_tag_name;

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

  /// The user-defined absolute tolerance for determining the convergence in passive scalars
  const std::vector<Real> _passive_scalar_absolute_tolerance;

  /// The maximum number of momentum-pressure iterations
  const unsigned int _num_iterations;

  /// Debug parameter which allows printing the coupling and solution vectors/matrices
  const bool _print_fields;
};
