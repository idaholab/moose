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
#include "MFEMFESpace.h"

/**
 * Wrapper for mfem::HypreADS solver.
 */
class MFEMHypreADS : public Moose::MFEM::LinearSolverBase, public Moose::MFEM::LORInterface
{
public:
  static InputParameters validParams();

  MFEMHypreADS(const InputParameters &);

  /// Updates the solver with the bilinear form in case LOR solve is required
  void SetupLOR() override;

  void Update() override
  {
    if (IsLOR(*this))
      SetupLOR();
  }

protected:
  void ConstructSolver() override;

private:
  const MFEMFESpace & _mfem_fespace;
};

#endif
