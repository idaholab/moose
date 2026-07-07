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

#include "MFEMLinearSolverBase.h"

/**
 * Wrapper for mfem::GMRESSolver.
 */
class MFEMGMRESSolver : public Moose::MFEM::LinearSolverBase, public Moose::MFEM::LORInterface
{
public:
  static InputParameters validParams();

  MFEMGMRESSolver(const InputParameters & parameters);

  /// Update the wrapped MFEM solver parameters
  virtual void SetSolverParameters(mfem::Solver & solver) override;

  void Update() override
  {
    Moose::MFEM::LinearSolverBase::Update();
    LORInterface::Update<mfem::GMRESSolver>(*this, *_equation_system);
  }

protected:
  void ConstructSolver() override;
};

#endif
