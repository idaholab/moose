#pragma once
#include "problem_builder_base.h"
#include "time_domain_problem_builder.h"
#include "time_domain_equation_system_problem_operator.h"
#include "equation_system_interface.h"

namespace platypus
{
/// Time-depent problems with an equation system.
class TimeDomainEquationSystemProblemData : public TimeDomainProblemData
{
  // public:
  //   TimeDomainEquationSystemProblemData() = default;
  //   ~TimeDomainEquationSystemProblemData() override = default;

  //   [[nodiscard]] platypus::TimeDomainEquationSystemProblemOperator * GetOperator() const
  //   override
  //   {
  //     return static_cast<platypus::TimeDomainEquationSystemProblemOperator *>(
  //         TimeDomainProblemData::GetOperator());
  //   }

  //   void
  //   SetOperator(std::unique_ptr<platypus::TimeDomainEquationSystemProblemOperator>
  //   problem_operator)
  //   {
  //     TimeDomainProblemData::SetOperator(std::move(problem_operator));
  //   }

  //   void ConstructOperator() override
  //   {
  //     auto equation_system = std::make_unique<platypus::TimeDependentEquationSystem>();
  //     auto problem_operator =
  //     std::make_unique<platypus::TimeDomainEquationSystemProblemOperator>(
  //         *this, std::move(equation_system));

  //     SetOperator(std::move(problem_operator));
  //   }

  //   [[nodiscard]] TimeDependentEquationSystem * GetEquationSystem() const override
  //   {
  //     return GetOperator()->GetEquationSystem();
  //   }
};

} // namespace platypus
