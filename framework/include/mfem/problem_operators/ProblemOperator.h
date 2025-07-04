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
#include "ProblemOperatorInterface.h"

namespace Moose::MFEM
{
/// Steady-state problem operator with no equation system.
class ProblemOperator : public mfem::Operator, public ProblemOperatorInterface
{
public:
  ProblemOperator(MFEMProblemData & problem) : ProblemOperatorInterface(problem) {}

  void SetGridFunctions() override;

  virtual void Solve(mfem::Vector & /*X*/) {}
  void Mult(const mfem::Vector &, mfem::Vector &) const override {}
};

} // namespace Moose::MFEM

#endif
