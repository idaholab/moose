#ifdef MFEM_ENABLED

#pragma once
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "time_domain_problem_operator.h"
#include "problem_operator_interface.h"
#include "equation_system_interface.h"

namespace MooseMFEM
{

/// Problem operator for time-dependent problems with an equation system.
class TimeDomainEquationSystemProblemOperator : public TimeDomainProblemOperator,
                                                public EquationSystemInterface
{
public:
  TimeDomainEquationSystemProblemOperator(MFEMProblemData & problem)
    : TimeDomainProblemOperator(problem),
      _equation_system(
          std::dynamic_pointer_cast<MooseMFEM::TimeDependentEquationSystem>(problem._eqn_system))
  {
  }

  void SetGridFunctions() override;
  void Init(mfem::BlockVector & X) override;

  void ImplicitSolve(const double dt, const mfem::Vector & X, mfem::Vector & dX_dt) override;

  [[nodiscard]] MooseMFEM::TimeDependentEquationSystem * GetEquationSystem() const override
  {
    if (!_equation_system)
    {
      MFEM_ABORT("No equation system has been added.");
    }

    return _equation_system.get();
  }

protected:
  void BuildEquationSystemOperator(double dt);

private:
  std::vector<mfem::ParGridFunction *> _trial_variable_time_derivatives;
  std::shared_ptr<MooseMFEM::TimeDependentEquationSystem> _equation_system{nullptr};
};

} // namespace MooseMFEM

#endif
