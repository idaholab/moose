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
 * Wrapper for mfem::BlockDiagonalPreconditioner that creates a mfem::BlockDiagonalPreconditioner
 * operator.
 *
 */
class BlockDiagonalPreconditioner : public mfem::Solver
{
public:
  void SetOperator(const mfem::Operator & op) override;
  void Mult(const mfem::Vector & x, mfem::Vector & y) const override;

  void SetBlockSolvers(std::vector<LinearSolverBase *> block_solvers)
  {
    _block_solvers = std::move(block_solvers);
  }

private:
  /// Solver for each diagonal block
  std::vector<LinearSolverBase *> _block_solvers;
  /// Block offsets of the current operator. mfem::BlockDiagonalPreconditioner
  /// keeps a reference to its offsets.
  mfem::Array<int> _offsets;
  std::unique_ptr<mfem::BlockDiagonalPreconditioner> _block_diag_precon;
};
} // namespace Moose::MFEM

/**
 * Wrapper for Moose::MFEM::BlockDiagonalPreconditioner
 */
class MFEMBlockDiagonalPreconditioner : public Moose::MFEM::LinearSolverBase
{
public:
  static InputParameters validParams();

  MFEMBlockDiagonalPreconditioner(const InputParameters &);

  void ConstructSolver() override;

protected:
  void UpdateEquationSystemContext() override;

private:
  /// Names of variables
  const std::vector<VariableName> & _variables;
  /// Preconditioner for each of the diagonal blocks in _variables
  std::vector<std::shared_ptr<Moose::MFEM::LinearSolverBase>> _block_preconditioners;
};

#endif
