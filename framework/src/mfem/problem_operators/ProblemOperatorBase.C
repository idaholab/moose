//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "ProblemOperatorBase.h"

class MFEMProblem;

namespace Moose::MFEM
{

ProblemOperatorBase::ProblemOperatorBase(MFEMProblem & problem)
  : _problem(problem), _problem_data(problem.getProblemData())
{
}

void
ProblemOperatorBase::SetGridFunctions()
{
  _trial_variables = _problem_data.gridfunctions.Get(_trial_var_names);
  _test_variables = _problem_data.gridfunctions.Get(_test_var_names);

  // Set operator size and block structure for trial spaces
  _block_true_offsets_trial.SetSize(_trial_variables.size() + 1);
  _block_true_offsets_trial[0] = 0;
  for (const auto ind : index_range(_trial_variables))
    _block_true_offsets_trial[ind + 1] = _trial_variables.at(ind)->ParFESpace()->TrueVSize();
  _block_true_offsets_trial.PartialSum();

  // Set operator size and block structure for test spaces
  _block_true_offsets_test.SetSize(_test_variables.size() + 1);
  _block_true_offsets_test[0] = 0;
  for (const auto ind : index_range(_test_variables))
    _block_true_offsets_test[ind + 1] = _test_variables.at(ind)->ParFESpace()->TrueVSize();
  _block_true_offsets_test.PartialSum();

  _true_x.Update(_block_true_offsets_trial);
  _true_rhs.Update(_block_true_offsets_test);
}

void
ProblemOperatorBase::Init(mfem::BlockVector & X)
{
  X.Update(_block_true_offsets_trial);
  for (const auto i : index_range(_trial_variables))
    X.GetBlock(i) = _trial_variables[i]->GetTrueVector();
  // Sync the flags from the global vector with the sub-vectors (copies to global vector location)
  X.SyncFromBlocks();

  // After initial assignment of X from the grid function, which may contain initial conditions,
  // we alias the grid function to X
  for (const auto i : index_range(_trial_variables))
    _trial_variables[i]->MakeTRef(
        _trial_variables[i]->ParFESpace(), X, _block_true_offsets_trial[i]);
  _trial_true_vector = &X;

  // This might seem silly but after making the tref the memory flags of the grid function and its
  // true vector are in an empty state other than the aliasing. This operation syncs the flags and
  // should be a no-op in terms of actual data transfer
  SetTrialVariablesFromTrueVectors();
}

void
ProblemOperatorBase::SetTrialVariablesFromTrueVectors()
{
  mooseAssert(_trial_true_vector, "The true vector should already have been set");
  for (const auto trial_var : _trial_variables)
  {
    // Sync the memory flags from the global true vector to the gridfunction aliases
    trial_var->GetTrueVector().SyncMemory(*_trial_true_vector);
    trial_var->SetFromTrueVector();
  }
}
}

#endif
