#pragma once
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMProblemData.h"
#include "problem_operator_interface.h"

namespace platypus
{
std::vector<std::string> GetTimeDerivativeNames(std::vector<std::string> gridfunction_names);

/// Problem operator for time-dependent problems with no equation system. The user will need to subclass this since the solve is not
/// implemented.
class TimeDomainProblemOperator : public mfem::TimeDependentOperator,
                                  public ProblemOperatorInterface
{
public:
  TimeDomainProblemOperator(MFEMProblemData & problem) : ProblemOperatorInterface(problem) {}
  ~TimeDomainProblemOperator() override = default;

  void SetGridFunctions() override;

  void ImplicitSolve(const double, const mfem::Vector &, mfem::Vector &) override {}
};

} // namespace platypus
