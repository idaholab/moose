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
#include "MFEMSolverBase.h"
#include "MFEMFESpace.h"

/**
 * Wrapper for mfem::HypreAMS solver.
 */
class MFEMHypreAMS : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMHypreAMS(const InputParameters &);

  /// Updates the solver with the bilinear form in case LOR solve is required
  void updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs) override;

protected:
  void constructSolver(const InputParameters & parameters) override;

private:
  const MFEMFESpace & _mfem_fespace;
};

#endif
