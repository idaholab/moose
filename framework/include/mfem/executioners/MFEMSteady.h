#pragma once
#include "MFEMExecutioner.h"
#include "steady_state_problem_builder.h"

class MFEMSteady : public MFEMExecutioner
{
public:
  static InputParameters validParams();

  MFEMSteady() = default;
  explicit MFEMSteady(const InputParameters & params);
  ~MFEMSteady() override = default;

  virtual void init() override;

  virtual void execute() override;

  platypus::SteadyStateProblemData * _problem{nullptr};

protected:
  Real _system_time;
  int & _time_step;
  Real & _time;

  /// Iteration number obtained from the main application
  unsigned int _output_iteration_number;

private:
  bool _last_solve_converged;
};