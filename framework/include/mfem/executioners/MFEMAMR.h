#ifdef MFEM_ENABLED

#pragma once

#include "MFEMExecutioner.h"
#include "EquationSystemProblemOperator.h"

class MFEMAMR : public MFEMExecutioner
{
public:
  static InputParameters validParams();

  explicit MFEMAMR(const InputParameters & params);
  ~MFEMAMR() override = default;

  void constructProblemOperator() override;
  virtual void init() override;
  virtual void execute() override;
  virtual bool addEstimator(std::shared_ptr<MFEMEstimator>) override;

protected:
  // Time variables used for consistency with MOOSE, needed for outputs.
  // Important for future synchronisation of solves in MultiApps
  Real _system_time;
  int & _time_step;
  Real & _time;

  /// Iteration number obtained from the main application
  unsigned int _output_iteration_number;

private:
  std::unique_ptr<Moose::MFEM::ProblemOperator> _problem_operator{nullptr};

  //! For now - let's just pass the stuff that was hardcoded directly in...
  std::string _test_var_name;
  std::string _kernel_name;
  std::string _fe_space_name;
};

#endif
