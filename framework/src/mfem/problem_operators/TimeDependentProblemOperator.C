//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "TimeDependentProblemOperator.h"

namespace Moose::MFEM
{
void
TimeDependentProblemOperator::SetGridFunctions()
{
  ProblemOperatorBase::SetGridFunctions();
  width = _block_true_offsets_trial[_trial_variables.size()];
  height = _block_true_offsets_test[_test_variables.size()];
}

} // namespace Moose::MFEM

#endif
