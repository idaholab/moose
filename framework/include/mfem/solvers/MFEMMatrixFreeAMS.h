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

namespace Moose::MFEM
{
/**
 * Wrapper for mfem::MatrixFreeAMS solver that creates an mfem::MatrixFreeAMS solver from the
 * operator when set.
 */
class MatrixFreeAMS : public mfem::Solver
{
public:
  MatrixFreeAMS(mfem::Coefficient & alpha_coef,
                mfem::Coefficient & beta_coef,
                int inner_pi_its = 0,
                int inner_g_its = 1);

  /// Set the bilinear form corresponding to the curl-curl problem being preconditioned
  void SetBilinearForm(mfem::ParBilinearForm & a) { _aform = &a; }
  /// Set the marker array labelling essential boundaries
  void SetBoundaryMarkers(mfem::Array<int> & ess_bdr_markers)
  {
    _ess_bdr_markers = ess_bdr_markers;
  }
  void SetOperator(const mfem::Operator & op) override;
  void Mult(const mfem::Vector & x, mfem::Vector & y) const override
  {
    _matrix_free_ams->Mult(x, y);
  }

private:
  std::unique_ptr<mfem::MatrixFreeAMS> _matrix_free_ams{nullptr};
  mfem::Coefficient & _alpha_coef;
  mfem::Coefficient & _beta_coef;
  const int _inner_pi_its;
  const int _inner_g_its;
  mfem::ParBilinearForm * _aform;
  mfem::Array<int> _ess_bdr_markers;
};
} // namespace Moose::MFEM

/**
 * Wrapper for mfem::MatrixFreeAMS solver.
 */
class MFEMMatrixFreeAMS : public Moose::MFEM::LinearSolverBase, public Moose::MFEM::LORInterface
{
public:
  static InputParameters validParams();

  MFEMMatrixFreeAMS(const InputParameters &);

  /// Updates the solver with the bilinear form, as MFEMMatrixFreeAMS is an LOR-based solver
  void SetupLOR() override;

  void Update() override
  {
    if (IsLOR(*this))
      SetupLOR();
  }

protected:
  void ConstructSolver() override;

private:
  mfem::Coefficient & _alpha_coef;
  mfem::Coefficient & _beta_coef;
  const int _inner_pi_its;
  const int _inner_g_its;
};

#endif
