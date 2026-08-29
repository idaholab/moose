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

#include <memory>

/**
 * Wrapper for mfem::HypreAMS solver.
 */
class MFEMHypreAMS : public Moose::MFEM::LORLinearSolverBase<mfem::HypreAMS>,
                     public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();

  MFEMHypreAMS(const InputParameters &);

protected:
  void ConstructSolver() override;

  /// Update the wrapped MFEM solver parameters
  virtual void SetSolverParameters(mfem::HypreAMS & solver) override;

  /// Builds the true-dof vector marking nodes interior to the zero-mass ("block") region,
  /// for use with HYPRE_AMSSetInteriorNodes. Only called when "block" is set by the user.
  std::unique_ptr<mfem::HypreParVector> BuildInteriorNodes();

private:
  const MFEMFESpace & _mfem_fespace;
  /// Number of iterations between projections onto the compatible H(curl) subspace (0 disables).
  const unsigned int _projection_frequency;
  /// True-dof marker (1.0 at interior nodes) passed to HYPRE_AMSSetInteriorNodes;
  /// null unless the "block" parameter is set by the user.
  const std::unique_ptr<mfem::HypreParVector> _interior_nodes;
};

#endif
