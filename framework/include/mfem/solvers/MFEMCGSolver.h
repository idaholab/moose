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

#include "MFEMLORLinearSolverBase.h"

/**
 * Wrapper for mfem::CGSolver.
 */
class MFEMCGSolver : public Moose::MFEM::LORLinearSolverBase<mfem::CGSolver>
{
public:
  static InputParameters validParams();

  MFEMCGSolver(const InputParameters & parameters);

  /// Update the wrapped MFEM solver parameters
  virtual void SetSolverParameters(mfem::CGSolver & solver) override;

protected:
  void ConstructSolver() override;  
};

#endif
