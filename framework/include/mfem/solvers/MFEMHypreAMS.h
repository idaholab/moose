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
#include "MFEMBlockRestrictable.h"

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

  /// Passes the interior-node marker to the solver once the essential boundaries are known.
  /// The solver keeps the marker across later operator updates.
  virtual void UpdateEquationSystemContext() override;

  /// Builds the true-dof vector marking nodes interior to the zero-mass ("block") region,
  /// for use with HYPRE_AMSSetInteriorNodes. Only called when restricted to subdomains.
  std::unique_ptr<mfem::HypreParVector> BuildInteriorNodes();

private:
  const MFEMFESpace & _mfem_fespace;
  /// True-dof marker (1.0 at interior nodes) passed to HYPRE_AMSSetInteriorNodes;
  /// null until the first operator update, and always null unless restricted to subdomains.
  std::unique_ptr<mfem::HypreParVector> _interior_nodes;
};

#endif
