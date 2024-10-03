#pragma once
#include "MFEMExecutioner.h"
#include "time_domain_equation_system_problem_operator.h"

class MFEMTransient : public MFEMExecutioner
{
public:
  static InputParameters validParams();

  MFEMTransient() = default;
  explicit MFEMTransient(const InputParameters & params);
  ~MFEMTransient() override = default;

  void Step(double dt, int it) const;
  virtual void init() override;
  virtual void execute() override;
  virtual void registerTimeDerivatives();

  [[nodiscard]] platypus::TimeDomainEquationSystemProblemOperator * GetOperator() const override
  {
    return static_cast<platypus::TimeDomainEquationSystemProblemOperator *>(
        _problem_operator.get());
  }

  void ConstructOperator() override
  {
    _problem_data._eqn_system = std::make_shared<platypus::TimeDependentEquationSystem>();
    auto problem_operator = std::make_unique<platypus::TimeDomainEquationSystemProblemOperator>(
        _problem_data,
        std::dynamic_pointer_cast<platypus::TimeDependentEquationSystem>(
            _problem_data._eqn_system));

    _problem_operator.reset();
    _problem_operator = std::move(problem_operator);
  }

  mutable double _t_step; // Time step

private:
  double _t_initial;       // Start time
  double _t_final;         // End time
  mutable double _t;       // Current time
  mutable int _it;         // Time index
  int _vis_steps;          // Number of cycles between each output update
  mutable bool _last_step; // Flag to check if current step is final
  std::unique_ptr<platypus::TimeDomainProblemOperator> _problem_operator{nullptr};
};
