#ifdef MFEM_ENABLED

#pragma once
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMProblemData.h"
#include "problem_operator_interface.h"

namespace MooseMFEM
{
/// Steady-state problem operator with no equation system.
class ProblemOperator : public mfem::Operator, public ProblemOperatorInterface
{
public:
  ProblemOperator(MFEMProblemData & problem) : ProblemOperatorInterface(problem) {}
  ~ProblemOperator() override = default;

  void SetGridFunctions() override;

  virtual void Solve(mfem::Vector & /*X*/) {}
  void Mult(const mfem::Vector &, mfem::Vector &) const override {}
};

} // namespace MooseMFEM

#endif
