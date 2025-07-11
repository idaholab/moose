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

class MFEMExecutioner
{
public:
  static InputParameters validParams();

  MFEMExecutioner(const InputParameters & params, MFEMProblem & mfem_problem);

  /// Returns a reference to the MFEMProblem instance.
  MFEMProblem & getMFEMProblem() { return _mfem_problem; }
  const MFEMProblem & getMFEMProblem() const { return _mfem_problem; }

  /// Virtual method to construct the ProblemOperator. Call for default problems.
  virtual void constructProblemOperator() = 0;

  /**
   * Perform all required solves during a step. Includes relevant methods from the libmesh-specific
   * FixedPointSolve::solve()
   */
  virtual void solve();

  /**
   * Perform all required solves during a step. Analagous to FixedPointSolve::innerSolve() for
   * libMesh problems
   */
  virtual void innerSolve() = 0;

protected:
  MFEMProblem & _mfem_problem;
  MFEMProblemData & _problem_data;
  mfem::Device _device;
};

#endif
