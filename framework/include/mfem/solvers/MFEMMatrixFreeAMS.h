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

namespace Moose::MFEM
{
/**
 * Wrapper for mfem::MatrixFreeAMS solver that creates a mfem::MatrixFreeAMS solver from the
 * operator when set.
 */
class MatrixFreeAMS : public mfem::Solver
{
public:
  MatrixFreeAMS(mfem::Coefficient * alpha_coef,
                mfem::Coefficient * beta_coef,
                const mfem::Array<int> & ess_bdr_markers,
                int inner_pi_its = 0,
                int inner_g_its = 1);

  void SetBilinearForm(mfem::ParBilinearForm & a) { _aform = &a; }
  void SetOperator(const mfem::Operator & op) override;
  void Mult(const mfem::Vector & x, mfem::Vector & y) const override
  {
    _matrix_free_ams->Mult(x, y);
  }

private:
  std::unique_ptr<mfem::MatrixFreeAMS> _matrix_free_ams{nullptr};
  mfem::ParBilinearForm * _aform;
  mfem::Coefficient * _alpha_coef;
  mfem::Coefficient * _beta_coef;
  const mfem::Array<int> & _ess_bdr_markers;
  const int _inner_pi_its;
  const int _inner_g_its;
};
} // namespace Moose::MFEM

/**
 * Wrapper for mfem::MatrixFreeAMS solver.
 */
class MFEMMatrixFreeAMS : public Moose::MFEM::LinearSolverBase, public MFEMBoundaryRestrictable
{
public:
  static InputParameters validParams();

  MFEMMatrixFreeAMS(const InputParameters &);

  /// Updates the solver with the bilinear form, as MFEMMatrixFreeAMS is an LOR-based solver
  void SetupLOR(mfem::ParBilinearForm & a, mfem::Array<int> & /*tdofs*/) override;

protected:
  void ConstructSolver() override;

private:
  mfem::ParBilinearForm * _aform;
  std::vector<BoundaryName> _ess_bdr_markerss;
  mfem::Coefficient & _alpha_coef;
  mfem::Coefficient & _beta_coef;
  const unsigned int _inner_pi_its;
  const unsigned int _inner_g_its;
};

#endif
