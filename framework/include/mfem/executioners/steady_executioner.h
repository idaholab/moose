#pragma once
#include "executioner_base.h"
#include "steady_state_problem_builder.h"

namespace platypus
{

class SteadyExecutioner : public Executioner
{
public:
  SteadyExecutioner() = default;
  explicit SteadyExecutioner(const InputParameters & params);

  void Solve() const override;

  void Execute() const override;

private:
  platypus::SteadyStateProblem * _problem{nullptr};
};

} // namespace platypus
