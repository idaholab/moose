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
/// Interface inherited by ProblemOperator and TimeDomainProblemOperator. Removes duplicated code in both classes.
class ProblemOperatorBase
{
public:
  ProblemOperatorBase(MFEMProblem & problem);
  virtual ~ProblemOperatorBase() = default;

  virtual void SetGridFunctions();
  virtual void SetTestVariablesFromTrueVectors();
  virtual void SetTrialVariablesFromTrueVectors();
  virtual void Init(mfem::BlockVector & X);
  virtual void Solve() = 0;
  virtual void UniformRefinement( int num_refinements=1 );

  virtual int GetProblemSize();

  mfem::Array<int> _block_true_offsets;
  mfem::Array<int> _global_block_true_offsets;

  mfem::BlockVector _true_x, _true_rhs;
  mfem::OperatorHandle _equation_system_operator;

protected:
  /// Reference to the current problem.
  MFEMProblem & _problem;
  MFEMProblemData & _problem_data;

  /// Vector of names of state gridfunctions used in formulation, ordered by appearance in block
  /// vector during solve.
  std::vector<std::string> _test_var_names;
  std::vector<mfem::ParGridFunction *> _test_variables;
  const mfem::Vector * _test_true_vector = nullptr;

  /// Vector of names of state gridfunctions used in formulation, ordered by appearance in block
  /// vector during solve.
  std::vector<std::string> _trial_var_names;
  std::vector<mfem::ParGridFunction *> _trial_variables;
};
}

#endif
