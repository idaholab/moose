#pragma once
#include "executioner_base.hpp"
#include "time_domain_problem_builder.hpp"

namespace hephaestus
{

class TransientExecutioner : public Executioner
{
public:
  mutable double _t_step; // Time step

  TransientExecutioner() = default;
  explicit TransientExecutioner(const hephaestus::InputParameters & params);

  ~TransientExecutioner() override = default;

  void Step(double dt, int it) const;

  void Solve() const override;

  void Execute() const override;

private:
  double _t_initial;       // Start time
  double _t_final;         // End time
  mutable double _t;       // Current time
  mutable int _it;         // Time index
  int _vis_steps;          // Number of cyces between each output update
  mutable bool _last_step; // Flag to check if current step is final
  hephaestus::TimeDomainProblem * _problem{nullptr};
};

} // namespace hephaestus
