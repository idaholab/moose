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
/// Interface inherited by ProblemOperator and TimeDependentProblemOperator. Removes duplicated code in both classes.
class ProblemOperatorBase
{
public:
  ProblemOperatorBase(Problem & problem);
  virtual ~ProblemOperatorBase() = default;

  virtual void SetGridFunctions();
  virtual void SetTrialVariablesFromTrueVectors();
  virtual void Init(mfem::BlockVector & X);
  virtual void Solve() = 0;

  mfem::Array<int> _block_true_offsets_test;
  mfem::Array<int> _block_true_offsets_trial;

  mfem::BlockVector _true_x, _true_rhs;

protected:
  /// Solve the current operator using the configured nonlinear solver, or a one-step Newton solve
  /// when no nonlinear solver object has been provided for a linear problem.
  void SolveWithOperator(mfem::Operator & op,
                         const mfem::Vector & rhs,
                         mfem::Vector & x,
                         bool nonlinear,
                         const std::function<void()> & prepare_linear_solver);

  /// Reference to the current problem.
  Problem & _problem;
  ProblemData & _problem_data;

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
