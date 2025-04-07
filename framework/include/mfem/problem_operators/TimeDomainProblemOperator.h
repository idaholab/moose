#ifdef MFEM_ENABLED

#pragma once
#include "MFEMProblemData.h"
#include "ProblemOperatorInterface.h"

namespace Moose::MFEM
{
std::vector<std::string> GetTimeDerivativeNames(std::vector<std::string> gridfunction_names);

/// Problem operator for time-dependent problems with no equation system. The user will need to subclass this since the solve is not
/// implemented.
class TimeDomainProblemOperator : public mfem::TimeDependentOperator,
                                  public ProblemOperatorInterface
{
public:
  TimeDomainProblemOperator(MFEMProblemData & problem) : ProblemOperatorInterface(problem) {}

  void SetGridFunctions() override;

  void ImplicitSolve(const double, const mfem::Vector &, mfem::Vector &) override {}
};

} // namespace Moose::MFEM

#endif
