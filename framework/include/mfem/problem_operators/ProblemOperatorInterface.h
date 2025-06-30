#ifdef MFEM_ENABLED

#pragma once
#include "MFEMProblemData.h"

namespace Moose::MFEM
{
/// Interface inherited by ProblemOperator and TimeDomainProblemOperator. Removes duplicated code in both classes.
class ProblemOperatorInterface
{
public:
  ProblemOperatorInterface(MFEMProblemData & problem) : _problem(problem) {}
  virtual ~ProblemOperatorInterface() = default;

  virtual void SetGridFunctions();
  virtual void SetTestVariablesFromTrueVectors();
  virtual void SetTrialVariablesFromTrueVectors();
  virtual void Init(mfem::BlockVector & X);

  virtual int  GetProblemSize();

  mfem::Array<int> _block_true_offsets;
  mfem::Array<int> _global_block_true_offsets;

  mfem::BlockVector _true_x, _true_rhs;
  mfem::OperatorHandle _equation_system_operator;

protected:
  // Reference to the current problem.
  MFEMProblemData & _problem;

  // Vector of names of state gridfunctions used in formulation, ordered by appearance in block
  // vector during solve.
  std::vector<std::string>             _test_var_names;
  std::vector<mfem::ParGridFunction *> _test_variables;

  // Vector of names of state gridfunctions used in formulation, ordered by appearance in block
  // vector during solve.
  std::vector<std::string>             _trial_var_names;
  std::vector<mfem::ParGridFunction *> _trial_variables;
};
}

#endif
