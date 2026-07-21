//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMProblem.h"
#include <functional>

namespace Moose::MFEM
{
class EquationSystem;

/**
 * Connects MFEMProblem's MOOSE solver objects to EquationSystem's mathematics.
 *
 * Three distinct layers collaborate to run an MFEM solve inside MOOSE:
 *
 *  - **MFEMProblem** owns MOOSE infrastructure: the object factory, the action system,
 *    ProblemData, and the configured solver objects.  Embedding the mfem::Operator
 *    solve-dispatch logic here would couple MOOSE's object-management layer directly
 *    to specific MFEM operator patterns, so it delegates to ProblemOperator instead.
 *
 *  - **EquationSystem** owns the mathematics: it builds weak-form components
 *    (bilinear/linear/nonlinear forms contributed by kernels and BCs), forms the constrained
 *    linear part, retains nonlinear action forms for residual/Jacobian evaluation, and exposes
 *    the resulting mfem::Operator interface consumed by both linear solvers (CG, GMRES, direct)
 *    and nonlinear solvers (Newton).  Solver selection is a user-configuration concern owned by
 *    MFEMProblem; coupling it into EquationSystem would conflate the mathematical description of
 *    a PDE with how MOOSE happens to solve it.
 *
 *  - **ProblemOperator** (this class) connects the two.  It provides:
 *      1. The steady vs. time-dependent dispatch boundary: all subclasses implement
 *         Solve() as the MOOSE-level entry point ("do whatever this step requires").
 *         Transient subclasses additionally inherit from mfem::TimeDependentOperator
 *         and implement ImplicitSolve(dt, t, x) - a different, MFEM-level contract
 *         that MFEM's ODE solvers (BackwardEuler, SDIRK, etc.) call internally.
 *         Solve() and ImplicitSolve() live at different abstraction layers and are
 *         not interchangeable; ImplicitSolve() carries dt and t precisely because
 *         it is the per-step callback in an ODE integration loop.
 *      2. SolveWithOperator() - the linear/nonlinear dispatch that requires knowing
 *         which MOOSE solver objects are configured (jacobian_solver, nonlinear_solver).
 *      3. Block-vector bookkeeping (trial/test true-DoF offsets and vectors) that
 *         bridges MFEM's true-DoF world with MOOSE's grid-function world.
 */
class ProblemOperatorBase
{
public:
  ProblemOperatorBase(MFEMProblem & problem);
  virtual ~ProblemOperatorBase() = default;

  virtual void SetGridFunctions();
  virtual void SetTrialVariablesFromTrueVectors();
  virtual void Init(mfem::BlockVector & X);
  virtual void Solve() = 0;

  mfem::Array<int> _block_true_offsets_test;
  mfem::Array<int> _block_true_offsets_trial;

  mfem::BlockVector _true_x, _true_rhs;

protected:
  /// Solve the current equation system/operator using the configured nonlinear solver or linear
  /// solver for a purely linear problem
  void
  SolveWithOperator(EquationSystem & equation_system, const mfem::Vector & rhs, mfem::Vector & x);

  /// Reference to the current problem.
  MFEMProblem & _problem;
  MFEMProblemData & _problem_data;

  /// Vector of names of state gridfunctions used in formulation, ordered by appearance in block
  /// vector during solve.
  std::vector<std::string> _trial_var_names;
  std::vector<std::string> _test_var_names;
  std::vector<mfem::ParGridFunction *> _trial_variables;
  std::vector<mfem::ParGridFunction *> _test_variables;
  mfem::Vector * _trial_true_vector = nullptr;
};
}

#endif
