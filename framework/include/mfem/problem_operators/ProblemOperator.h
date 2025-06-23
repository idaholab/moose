#ifdef MFEM_ENABLED

#pragma once
#include "MFEMProblemData.h"
#include "ProblemOperatorInterface.h"
#include "MFEMEstimator.h"

namespace Moose::MFEM
{
/// Steady-state problem operator with no equation system.
class ProblemOperator : public mfem::Operator, public ProblemOperatorInterface
{
public:
  ProblemOperator(MFEMProblemData & problem) : ProblemOperatorInterface(problem) {}

  void SetGridFunctions() override;

  /*
  These functions are for doing AMR stuff with - however we only want this to happen in
  the EquationSystemProblemOperator. Since that one inherits from this class, we need
  to add trivial virtual functions up here to make that possible.
  */
  virtual void AddEstimator(std::shared_ptr<MFEMEstimator> /*estimator*/) {}
  virtual void SetUpAMR() {};
  virtual bool HRefine() { return false; } /* we return true when it's time to stop solving */
  virtual bool PRefine()
  {
    return false;
  } /* we return true when it's time to stop solving */

  virtual void Solve(mfem::Vector & /*X*/) {}
  void Mult(const mfem::Vector &, mfem::Vector &) const override {}
};

} // namespace Moose::MFEM

#endif
