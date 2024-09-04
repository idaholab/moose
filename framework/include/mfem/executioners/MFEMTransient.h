#pragma once
#include "MFEMExecutioner.h"
#include "time_domain_problem_builder.h"

class MFEMTransient : public MFEMExecutioner
{
public:
  static InputParameters validParams();

  MFEMTransient() = default;
  explicit MFEMTransient(const InputParameters & params);
  ~MFEMTransient() override = default;

  void Step(double dt, int it) const;

  void Solve() const override;

  void Execute() const override;

  virtual void init() override;

  virtual void execute() override;

  platypus::TimeDomainProblem * _problem{nullptr};
  mutable double _t_step; // Time step

private:
  double _t_initial;       // Start time
  double _t_final;         // End time
  mutable double _t;       // Current time
  mutable int _it;         // Time index
  int _vis_steps;          // Number of cyces between each output update
  mutable bool _last_step; // Flag to check if current step is final
};
