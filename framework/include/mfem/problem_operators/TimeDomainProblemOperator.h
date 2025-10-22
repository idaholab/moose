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
#include "MFEMProblemData.h"
#include "ProblemOperatorBase.h"

namespace Moose::MFEM
{
/// Problem operator for time-dependent problems with no equation system. The user will need to subclass this since the solve is not
/// implemented.
class TimeDomainProblemOperator : public mfem::TimeDependentOperator, public ProblemOperatorBase
{
public:
  TimeDomainProblemOperator(MFEMProblem & problem) : ProblemOperatorBase(problem) {}

  void SetGridFunctions() override;
  void Solve() override {}
  void ImplicitSolve(const mfem::real_t, const mfem::Vector &, mfem::Vector &) override {}
};

} // namespace Moose::MFEM

#endif
