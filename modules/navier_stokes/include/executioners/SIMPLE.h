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

  std::vector<NonlinearSystemBase *> & getMomentumSystem() { return _momentum_systems; }
  Real getMomentumRelaxation() { return _momentum_equation_relaxation; }

  const INSFVRhieChowInterpolatorSegregated & getRCUserObject() { return *_rc_uo; }

  PetscReal computeNormalizationFactor(const PetscVector<Number> & solution,
                                       const PetscMatrix<Number> & mat,
                                       const PetscVector<Number> & rhs);

protected:
  void relaxMatrix(SparseMatrix<Number> & matrix_in,
                   const Real relaxation_parameter,
                   NumericVector<Number> & diff_diagonal);

  void relaxRightHandSide(NumericVector<Number> & rhs_in,
                          const NumericVector<Number> & solution_in,
                          const NumericVector<Number> & diff_diagonal);

  std::vector<Real> solveMomentumPredictor(std::vector<NonlinearSystemBase *> & momentum_system);
  Real solvePressureCorrector(NonlinearSystemBase & pressure_system_in);
  void relaxPressureUpdate(NonlinearSystemBase & pressure_system_in);

  bool converged(const std::vector<Real> & momentum_residuals, const Real pressure_residual);

  PetscReal _norm_factor;

  FEProblemBase & _problem;

  FEProblemSolve _feproblem_solve;

  Real _system_time;
  int & _time_step;
  Real & _time;

  ///
  const std::vector<std::string> _momentum_system_names;
  /// The number of the nonlinear system corresponding to the momentum equation
  std::vector<unsigned int> _momentum_system_numbers;
  /// The number of the nonlinear system corresponding to the pressure equation
  const unsigned int _pressure_sys_number;
  /// Reference to the nonlinear system corresponding to the momentum equation
  std::vector<NonlinearSystemBase *> _momentum_systems;
  /// Reference to the nonlinear system corresponding to the pressure equation
  NonlinearSystemBase & _pressure_system;

  INSFVRhieChowInterpolatorSegregated * _rc_uo;

private:
  bool _last_solve_converged;
  const std::string _momentum_tag_name;
  const TagID _momentum_tag_id;

  const Real _momentum_equation_relaxation;
  const Real _pressure_variable_relaxation;

  const Real _momentum_absolute_tolerance;
  const Real _pressure_absolute_tolerance;

  const unsigned int _num_iterations;
  const bool _print_fields;
};
