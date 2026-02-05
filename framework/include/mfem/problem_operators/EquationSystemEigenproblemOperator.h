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

#include "EquationSystemProblemOperator.h"

namespace Moose::MFEM
{
/// Steady-state problem operator with an equation system.
class EquationSystemEigenproblemOperator : public EquationSystemProblemOperator
{
public:
  EquationSystemEigenproblemOperator(MFEMProblem & problem)
    : EquationSystemProblemOperator(problem)
  {
  }

  virtual void Solve() override;

  mfem::OperatorHandle _mass_rhs;

};

} // namespace Moose::MFEM

#endif
