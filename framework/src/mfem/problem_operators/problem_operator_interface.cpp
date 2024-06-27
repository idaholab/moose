#include "problem_operator_interface.hpp"

namespace hephaestus
{
void
ProblemOperatorInterface::SetGridFunctions()
{
  _trial_variables = _problem._gridfunctions.Get(_trial_var_names);

  // Set operator size and block structure
  _block_true_offsets.SetSize(_trial_variables.size() + 1);
  _block_true_offsets[0] = 0;
  for (unsigned int ind = 0; ind < _trial_variables.size(); ++ind)
  {
    _block_true_offsets[ind + 1] = _trial_variables.at(ind)->ParFESpace()->TrueVSize();
  }
  _block_true_offsets.PartialSum();

  _true_offsets.SetSize(_trial_variables.size() + 1);
  _true_offsets[0] = 0;
  for (unsigned int ind = 0; ind < _trial_variables.size(); ++ind)
  {
    _true_offsets[ind + 1] = _trial_variables.at(ind)->ParFESpace()->GetVSize();
  }
  _true_offsets.PartialSum();

  _true_x.Update(_block_true_offsets);
  _true_rhs.Update(_block_true_offsets);
}

void
ProblemOperatorInterface::Init(mfem::Vector & X)
{
  for (size_t i = 0; i < _trial_variables.size(); ++i)
  {
    mfem::ParGridFunction * trial_var = _trial_variables.at(i);

    trial_var->MakeRef(trial_var->ParFESpace(), const_cast<mfem::Vector &>(X), _true_offsets[i]);
    *trial_var = 0.0;
  }
}

}