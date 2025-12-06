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

namespace Moose::MFEM
{
/// Interface inherited by ProblemOperator and TimeDependentProblemOperator. Removes duplicated code in both classes.
class ProblemOperatorBase
{
public:
  ProblemOperatorBase(MFEMProblem & problem);
  virtual ~ProblemOperatorBase() = default;

  virtual void SetGridFunctions();
  virtual void SetTrialVariablesFromTrueVectors();
  virtual void Init(mfem::BlockVector & X);
  virtual void Solve() = 0;

  mfem::Array<int> _block_true_offsets;

  mfem::BlockVector _true_x, _true_rhs;

protected:
  /// Reference to the current problem.
  MFEMProblem & _problem;
  MFEMProblemData & _problem_data;

  /// Vector of names of state gridfunctions used in formulation, ordered by appearance in block
  /// vector during solve.
  std::vector<std::string> _trial_var_names;
  std::vector<mfem::ParGridFunction *> _trial_variables;
  mfem::Vector * _trial_true_vector = nullptr;
};
}

#endif
