#pragma once
#include "executioner_base.h"
#include "steady_state_problem_builder.h"

namespace hephaestus
{

class SteadyExecutioner : public Executioner
{
public:
  SteadyExecutioner() = default;
  explicit SteadyExecutioner(const hephaestus::InputParameters & params);

  void Solve() const override;

  void Execute() const override;

private:
  hephaestus::SteadyStateProblem * _problem{nullptr};
};

} // namespace hephaestus
