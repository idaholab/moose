//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "TimeDomainProblemOperator.h"

namespace Moose::MFEM
{
void
TimeDomainProblemOperator::SetGridFunctions()
{
  ProblemOperatorBase::SetGridFunctions();
  width = height = _block_true_offsets[_trial_variables.size()];
}

} // namespace Moose::MFEM

#endif
