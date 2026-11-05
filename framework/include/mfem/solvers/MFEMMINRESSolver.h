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
 * Wrapper for mfem::MINRESSolver.
 */
class MFEMMINRESSolver : public Moose::MFEM::LORLinearSolverBase<mfem::MINRESSolver>
{
public:
  static InputParameters validParams();

  MFEMMINRESSolver(const InputParameters & parameters);

  void ConstructSolver() override;

protected:
  /// Update the wrapped MFEM solver parameters
  virtual void SetSolverParameters(mfem::MINRESSolver & solver) override;

  virtual void SetPreconditionerImpl() override
  {
    cast_ref<mfem::MINRESSolver &>(GetSolver()).SetPreconditioner(_preconditioner->GetSolver());
  }
};

#endif
