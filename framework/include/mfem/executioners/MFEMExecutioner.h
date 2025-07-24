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
#include "MFEMProblemData.h"

class MFEMProblem;

class MFEMExecutioner : public Executioner
{
public:
  static InputParameters validParams();

  MFEMExecutioner(const InputParameters & params);

  virtual bool lastSolveConverged() const override { return true; };

  /// Virtual method to construct the ProblemOperator. Call for default problems.
  virtual void constructProblemOperator() = 0;

protected:
  MFEMProblem & _mfem_problem;
  MFEMProblemData & _problem_data;
};

#endif
