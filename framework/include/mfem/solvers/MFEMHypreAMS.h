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
#include "MFEMFESpace.h"

/**
 * Wrapper for mfem::HypreAMS solver.
 */
class MFEMHypreAMS : public Moose::MFEM::LORLinearSolverBase<mfem::HypreAMS>
{
public:
  static InputParameters validParams();

  MFEMHypreAMS(const InputParameters &);

  /// Update the wrapped MFEM solver parameters
  virtual void SetSolverParameters(mfem::HypreAMS & solver) override;

protected:
  void ConstructSolver() override;

private:
  const MFEMFESpace & _mfem_fespace;
};

#endif
