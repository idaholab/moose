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
 * Wrapper for mfem::GMRESSolver.
 */
class MFEMGMRESSolver : public Moose::MFEM::LORLinearSolverBase<mfem::GMRESSolver>
{
public:
  static InputParameters validParams();

  MFEMGMRESSolver(const InputParameters & parameters);

protected:
  void ConstructSolver() override;

  /// Update the wrapped MFEM solver parameters
  virtual void SetSolverParameters(mfem::GMRESSolver & solver) override;
};

#endif
