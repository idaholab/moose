#ifdef MFEM_ENABLED

#pragma once
#include "ProblemOperator.h"
#include "EquationSystemInterface.h"

namespace MooseMFEM
{
/// Steady-state problem operator with an equation system.
class EquationSystemProblemOperator : public ProblemOperator, public EquationSystemInterface
{
public:
  EquationSystemProblemOperator(MFEMProblemData & problem)
    : ProblemOperator(problem), _equation_system(problem.eqn_system)
  {
  }

  void SetGridFunctions() override;
  void Init(mfem::BlockVector & X) override;
  virtual void Solve(mfem::Vector & X) override;

  ~EquationSystemProblemOperator() override = default;

  [[nodiscard]] MooseMFEM::EquationSystem * GetEquationSystem() const override
  {
    if (!_equation_system)
    {
      MFEM_ABORT("No equation system has been added to ProblemOperator.");
    }

    return _equation_system.get();
  }

private:
  std::shared_ptr<MooseMFEM::EquationSystem> _equation_system{nullptr};
};

} // namespace MooseMFEM

#endif
