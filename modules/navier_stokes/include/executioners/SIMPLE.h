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

  /// Compute a normalizan factor which is applied to the linear residual to determine convergence
  PetscReal computeNormalizationFactor(const NumericVector<Number> & solution,
                                       const SparseMatrix<Number> & mat,
                                       const NumericVector<Number> & rhs);

protected:
  /// Relax the matrix to ensure diagonal dominance, we hold onto the difference in diagonals
  /// for later use in relaxing the right hand side
  void relaxMatrix(SparseMatrix<Number> & matrix_in,
                   const Real relaxation_parameter,
                   NumericVector<Number> & diff_diagonal);

  /// Relax the right hand side of an equation, this needs to be called once the
  /// field describing the difference in the diagonals is already available.
  void relaxRightHandSide(NumericVector<Number> & rhs_in,
                          const NumericVector<Number> & solution_in,
                          const NumericVector<Number> & diff_diagonal);

  /// Solve a momentum predctor step with a fixed pressure field
  std::vector<Real> solveMomentumPredictor();

  /// Solve a pressure corrector step
  Real solvePressureCorrector();

  /// Relax the update on the pressure.
  void relaxPressureUpdate(NonlinearSystemBase & pressure_system_in);

  /// Determine if the iterative process converged or not
  bool converged(const std::vector<Real> & momentum_residuals, const Real pressure_residual);

  FEProblemBase & _problem;
  FEProblemSolve _feproblem_solve;
  Real _system_time;
  int & _time_step;
  Real & _time;

  /// The names of the momentum systems. If only one provided we assume that the
  /// simulation is monolithic in terms of the momentum components.
  const std::vector<std::string> _momentum_system_names;

  /// The number of the system(s) corresponding to the momentum equation(s)
  std::vector<unsigned int> _momentum_system_numbers;

  /// The number of the system corresponding to the pressure equation
  const unsigned int _pressure_sys_number;

  /// Pointer(s) to the system(s) corresponding to the momentum equation(s)
  std::vector<NonlinearSystemBase *> _momentum_systems;

  /// Reference to the nonlinear system corresponding to the pressure equation
  NonlinearSystemBase & _pressure_system;

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

  /// The user-defined absolute tolerance for determining the convergence in momentum
  const Real _momentum_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in pressure
  const Real _pressure_absolute_tolerance;

  /// The maximum number of momentum-pressure iterations
  const unsigned int _num_iterations;

  /// Debug parameter which allows printing the coupling and solution vectors/matrices
  const bool _print_fields;
};
