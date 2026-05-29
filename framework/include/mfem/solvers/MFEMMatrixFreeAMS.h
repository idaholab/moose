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
#include "MFEMBoundaryRestrictable.h"

/**
 * Wrapper for mfem::HypreAMS solver.
 */
class MFEMMatrixFreeAMS : public MFEMSolverBase, public MFEMBoundaryRestrictable
{
public:
  static InputParameters validParams();

  MFEMMatrixFreeAMS(const InputParameters &);

  /// The constructor of mfem::MatrixFreeAMS requires the target operator to be known, so this constructs the solver
  virtual void setOperator(mfem::OperatorHandle & op) override;
  /// Updates the solver with the bilinear form in case LOR solve is required
  void updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs) override;

protected:
  void constructSolver() override{};

private:
  mfem::ParBilinearForm * _aform;
  std::vector<BoundaryName> _ess_bdrs;
};

#endif
