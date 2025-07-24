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
#include "MFEMProblemOperatorInterface.h"
#include "MFEMProblemSolve.h"
#include "EquationSystemProblemOperator.h"

class MFEMSteady : public Executioner, public Moose::MFEM::MFEMProblemOperatorInterface
{
public:
  static InputParameters validParams();

  explicit MFEMSteady(const InputParameters & params);

  virtual void init() override;
  virtual void execute() override;

  /// Check if last solve converged. 
  virtual bool lastSolveConverged() const override { return _last_solve_converged; };

private:
  MFEMProblem & _mfem_problem;
  MFEMProblemData & _mfem_problem_data;
  MFEMProblemSolve _mfem_problem_solve;

  // Time variables used for consistency with MOOSE, needed for outputs.
  // Important for future synchronisation of solves in MultiApps
  Real _system_time;
  int & _time_step;
  Real & _time;

  /// Flag showing if the last solve converged
  bool _last_solve_converged;
};

#endif
