//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "ProblemOperatorInterface.h"

namespace Moose::MFEM
{
void
ProblemOperatorInterface::SetGridFunctions()
{
  _test_variables = _problem.gridfunctions.Get(_test_var_names);
  _trial_variables = _problem.gridfunctions.Get(_trial_var_names);

  // Set operator size and block structure
  _block_true_offsets.SetSize(_trial_variables.size() + 1);
  _block_true_offsets[0] = 0;
  for (unsigned int ind = 0; ind < _trial_variables.size(); ++ind)
  {
    _block_true_offsets[ind + 1] = _trial_variables.at(ind)->ParFESpace()->TrueVSize();
  }
  _block_true_offsets.PartialSum();

  _true_x.Update(_block_true_offsets);
  _true_rhs.Update(_block_true_offsets);
}

void
ProblemOperatorInterface::Init(mfem::BlockVector & X)
{
  X.Update(_block_true_offsets);
  for (size_t i = 0; i < _test_variables.size(); ++i)
  {
    auto * const test_var = _test_variables.at(i);
    X.GetBlock(i) = (*test_var);
    // Sync the flags from sub-block vectors to global vector
    X.SyncFromBlocks();

    // After initial assignment of X from the grid function, which may contain initial conditions,
    // we alias the grid function to X
    test_var->MakeTRef(_test_variables.at(i)->ParFESpace(), X, _block_true_offsets[i]);
    _test_true_vector = &X;
    // This might seem silly but after making the tref the memory flags of the grid function and its
    // true vector are in an empty state other than the aliasing. This operation syncs the flags and
    // should be a no-op in terms of actual data transfer
    test_var->GetTrueVector().SyncMemory(*_test_true_vector);
    test_var->SetFromTrueVector();
  }
}

void
ProblemOperatorInterface::SetTestVariablesFromTrueVectors()
{
  for (unsigned int ind = 0; ind < _test_variables.size(); ++ind)
  {
    auto * const test_var = _test_variables.at(ind);

    // We must sync the memory flags from the true true vector to the grid function copy of the true
    // vector
    mooseAssert(_test_true_vector, "The true vector should already have been set");
    test_var->GetTrueVector().SyncMemory(*_test_true_vector);
    test_var->SetFromTrueVector();
  }
}

void
ProblemOperatorInterface::SetTrialVariablesFromTrueVectors()
{
  for (unsigned int ind = 0; ind < _trial_variables.size(); ++ind)
  {
    _trial_variables.at(ind)->SetFromTrueVector();
  }
}

}

#endif
