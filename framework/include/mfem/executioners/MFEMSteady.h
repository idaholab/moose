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
#include "MFEMExecutioner.h"
#include "EquationSystemProblemOperator.h"

class MFEMSteady : public Executioner
{
public:
  static InputParameters validParams();

  explicit MFEMSteady(const InputParameters & params);

  void constructProblemOperator();
  virtual Moose::MFEM::ProblemOperator & getProblemOperator() { return *_problem_operator; };

  virtual void init() override;
  virtual void execute() override;

  /// Check if last solve converged. Currently defaults to true for all MFEM executioners.
  virtual bool lastSolveConverged() const override { return true; };

protected:
  // Time variables used for consistency with MOOSE, needed for outputs.
  // Important for future synchronisation of solves in MultiApps
  Real _system_time;
  int & _time_step;
  Real & _time;

  /// Iteration number obtained from the main application
  unsigned int _output_iteration_number;

private:
  MFEMProblem & _mfem_problem;
  MFEMProblemData & _mfem_problem_data;
  MFEMExecutioner _mfem_problem_solver;
  std::unique_ptr<Moose::MFEM::ProblemOperator> _problem_operator{nullptr};
};

#endif
