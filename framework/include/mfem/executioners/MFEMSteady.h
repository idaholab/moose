//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "Executioner.h"
#include "ProblemOperatorInterface.h"
#include "MFEMProblemSolve.h"
#include "EquationSystemProblemOperator.h"
#include "ComplexEquationSystemProblemOperator.h"

namespace Moose::MFEM
{
class Steady : public Executioner, public ProblemOperatorInterface
{
public:
  static InputParameters validParams();

  explicit Steady(const InputParameters & params);

  virtual void init() override;
  virtual void execute() override;

  /// Check if last solve converged.
  virtual bool lastSolveConverged() const override { return _last_solve_converged; };

private:
  Problem & _mfem_problem;
  ProblemData & _mfem_problem_data;
  ProblemSolve _mfem_problem_solve;

  // Time variables used for consistency with MOOSE, needed for outputs.
  // Important for future synchronisation of solves in MultiApps
  Real _system_time;
  int & _time_step;
  Real & _time;

  /// Flag showing if the last solve converged
  bool _last_solve_converged;
};

} // namespace Moose::MFEM
#endif
