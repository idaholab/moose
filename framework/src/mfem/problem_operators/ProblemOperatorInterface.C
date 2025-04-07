#ifdef MFEM_ENABLED

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
  X = 0.0;
  for (size_t i = 0; i < _test_variables.size(); ++i)
  {
    _test_variables.at(i)->MakeTRef(_test_variables.at(i)->ParFESpace(), X, _block_true_offsets[i]);
  }
}

void
ProblemOperatorInterface::SetTestVariablesFromTrueVectors()
{
  for (unsigned int ind = 0; ind < _test_variables.size(); ++ind)
  {
    _test_variables.at(ind)->SetFromTrueVector();
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
