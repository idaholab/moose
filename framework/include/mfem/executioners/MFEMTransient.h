#ifdef MFEM_ENABLED

#pragma once
#include "MFEMExecutioner.h"
#include "time_domain_equation_system_problem_operator.h"

class MFEMTransient : public MFEMExecutioner
{
public:
  static InputParameters validParams();

  explicit MFEMTransient(const InputParameters & params);
  ~MFEMTransient() override = default;

  void constructProblemOperator() override;
  void step(double dt, int it) const;
  virtual void init() override;
  virtual void execute() override;

  mutable double _t_step; // Time step

private:
  double _t_initial;       // Start time
  double _t_final;         // End time
  Real & _t;               // Current time
  mutable int _it;         // Time index
  int _vis_steps;          // Number of cycles between each output update
  mutable bool _last_step; // Flag to check if current step is final
  std::unique_ptr<MooseMFEM::TimeDomainProblemOperator> _problem_operator{nullptr};
};

#endif
