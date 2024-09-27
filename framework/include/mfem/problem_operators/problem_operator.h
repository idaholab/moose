#pragma once
#include "../common/pfem_extras.hpp"
#include "problem_builder_base.h"
#include "problem_operator_interface.h"

namespace platypus
{
/// Steady-state problem operator with no equation system.
class ProblemOperator : public mfem::Operator, public ProblemOperatorInterface
{
public:
  ProblemOperator(platypus::ProblemData & problem) : ProblemOperatorInterface(problem) {}
  ~ProblemOperator() override = default;

  void SetGridFunctions() override;

  virtual void Solve(mfem::Vector & X) {}
  void Mult(const mfem::Vector & x, mfem::Vector & y) const override {}
};

} // namespace platypus