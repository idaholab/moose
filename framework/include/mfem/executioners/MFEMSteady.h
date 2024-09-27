#pragma once
#include "MFEMExecutioner.h"
#include "equation_system_problem_operator.h"

class MFEMSteady : public MFEMExecutioner
{
public:
  static InputParameters validParams();

  MFEMSteady() = default;
  explicit MFEMSteady(const InputParameters & params);
  ~MFEMSteady() override = default;

  virtual void init() override;

  virtual void execute() override;

  platypus::ProblemData * _problem{nullptr};

  [[nodiscard]] platypus::EquationSystemProblemOperator * GetOperator() const override
  {
    return static_cast<platypus::EquationSystemProblemOperator *>(_problem_operator.get());
  }

  [[nodiscard]] platypus::EquationSystem * GetEquationSystem() const override
  {
    return GetOperator()->GetEquationSystem();
  }

  void SetOperator(std::unique_ptr<platypus::EquationSystemProblemOperator> problem_operator)
  {
    _problem_operator.reset();
    _problem_operator = std::move(problem_operator);
  }

  void ConstructOperator() override
  {
    _problem = &_mfem_problem.getProblemData();
    _problem->_eqn_system = std::make_shared<platypus::EquationSystem>();
    auto problem_operator =
        std::make_unique<platypus::EquationSystemProblemOperator>(*_problem, _problem->_eqn_system);

    SetOperator(std::move(problem_operator));
  }

protected:
  Real _system_time;
  int & _time_step;
  Real & _time;

  /// Iteration number obtained from the main application
  unsigned int _output_iteration_number;

private:
  bool _last_solve_converged;
  std::unique_ptr<platypus::ProblemOperator> _problem_operator{nullptr};
};