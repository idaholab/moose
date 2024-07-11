#pragma once
#include "executioner_base.h"
#include "time_domain_problem_builder.h"

namespace platypus
{

class TransientExecutioner : public Executioner
{
public:
  mutable double _t_step; // Time step

  TransientExecutioner() = default;
  explicit TransientExecutioner(const platypus::InputParameters & params);

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
  platypus::TimeDomainProblem * _problem{nullptr};
};

} // namespace platypus
