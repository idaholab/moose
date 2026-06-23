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
#include "MFEMBoundaryRestrictable.h"

/**
 * Wrapper for mfem::HypreAMS solver.
 */
class MFEMMatrixFreeAMS : public Moose::MFEM::LinearSolverBase, public MFEMBoundaryRestrictable
{
public:
  static InputParameters validParams();

  MFEMMatrixFreeAMS(const InputParameters &);

  /// The constructor of mfem::MatrixFreeAMS requires the target operator to be known, so this constructs the solver
  virtual void SetOperator(mfem::OperatorHandle & op) override;
  /// Updates the solver with the bilinear form, as MFEMMatrixFreeAMS is an LOR-based solver
  void SetupLOR(mfem::ParBilinearForm & a, mfem::Array<int> & /*tdofs*/) override;

protected:
  void ConstructSolver() override{};

private:
  mfem::ParBilinearForm * _aform;
  std::vector<BoundaryName> _ess_bdrs;
};

#endif
