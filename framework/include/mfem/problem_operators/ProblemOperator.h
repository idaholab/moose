#ifdef MFEM_ENABLED

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
