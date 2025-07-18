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
#include "ProblemOperatorInterface.h"

class MFEMProblem;

class MFEMExecutioner
{
public:
  static InputParameters validParams();

  MFEMExecutioner(const InputParameters & params, MFEMProblem & mfem_problem);

  /**
   * Perform all required solves during a step. Includes relevant methods from the libmesh-specific
   * FixedPointSolve::solve() for one iteration.
   */
  virtual void solve(Moose::MFEM::ProblemOperatorInterface & problem_operator);

protected:
  MFEMProblem & _mfem_problem;
  mfem::Device _device;
};

#endif
