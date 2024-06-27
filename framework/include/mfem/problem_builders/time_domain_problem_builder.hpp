#pragma once
#include "problem_builder_base.hpp"
#include "time_domain_problem_operator.hpp"

namespace hephaestus
{

/// Time-dependent problems with no equation system.
class TimeDomainProblem : public Problem
{
public:
  [[nodiscard]] hephaestus::TimeDomainProblemOperator * GetOperator() const override
  {
    if (!_problem_operator)
    {
      MFEM_ABORT("No operator has been added.");
    }

    return _problem_operator.get();
  }

  void SetOperator(std::unique_ptr<hephaestus::TimeDomainProblemOperator> problem_operator)
  {
    _problem_operator.reset();
    _problem_operator = std::move(problem_operator);
  }

  void ConstructOperator() override
  {
    _problem_operator.reset();
    _problem_operator = std::make_unique<hephaestus::TimeDomainProblemOperator>(*this);
  }

private:
  std::unique_ptr<hephaestus::TimeDomainProblemOperator> _problem_operator{nullptr};
};

/// Problem-builder for TimeDomainProblem.
class TimeDomainProblemBuilder : public ProblemBuilder
{
public:
  TimeDomainProblemBuilder() : ProblemBuilder(new hephaestus::TimeDomainProblem) {}

  ~TimeDomainProblemBuilder() override = default;

  auto ReturnProblem() { return ProblemBuilder::ReturnProblem<TimeDomainProblem>(); }

  static std::vector<mfem::ParGridFunction *>
  RegisterTimeDerivatives(std::vector<std::string> gridfunction_names,
                          hephaestus::GridFunctions & gridfunctions);

  void RegisterFESpaces() override {}

  void RegisterGridFunctions() override;

  void RegisterAuxSolvers() override {}

  void RegisterCoefficients() override {}

  void SetOperatorGridFunctions() override;

  void ConstructOperator() override;

  void ConstructState() override;

  void ConstructTimestepper() override;

protected:
  /// NB: constructor called in derived classes.
  TimeDomainProblemBuilder(hephaestus::TimeDomainProblem * problem) : ProblemBuilder(problem) {}

  [[nodiscard]] hephaestus::TimeDomainProblem * GetProblem() const override
  {
    return ProblemBuilder::GetProblem<hephaestus::TimeDomainProblem>();
  };
};

} // namespace hephaestus
