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
 * Wrapper for mfem::HyprePCG solver.
 */
class MFEMHyprePCG : public Moose::MFEM::LORLinearSolverBase<mfem::HyprePCG>
{
public:
  static InputParameters validParams();

  MFEMHyprePCG(const InputParameters & parameters);

  /// Update the wrapped MFEM solver parameters
  virtual void SetSolverParameters(mfem::HyprePCG & solver) override;

  void Update() override
  {
    SetupLOR(*this, *_equation_system);
    // Reset preconditioner if it has had an LOR update
    if (IsLOR(*this) && !_lor)
      SetPreconditioner(static_cast<mfem::HyprePCG &>(GetSolver()));
  }

protected:
  void ConstructSolver() override;
};

#endif
