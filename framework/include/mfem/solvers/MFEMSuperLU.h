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
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"
#include <memory>

namespace Moose::MFEM
{

/**
 * Wrapper for mfem::SuperLUSolver that creates a SuperLURowLocMatrix from the operator when set.
 */
class SuperLUSolver : public mfem::SuperLUSolver
{
public:
  SuperLUSolver(MPI_Comm comm, int npdep = 1)
    : mfem::SuperLUSolver(comm), _s_superlu(std::make_unique<mfem::SuperLUSolver>(comm, npdep)) {};
  void SetOperator(const mfem::Operator & op) override
  {
    _a_superlu = std::make_unique<mfem::SuperLURowLocMatrix>(op);
    _s_superlu->SetOperator(*_a_superlu.get());
  }
  void Mult(const mfem::Vector & x, mfem::Vector & y) const override { _s_superlu->Mult(x, y); }

private:
  std::unique_ptr<mfem::SuperLURowLocMatrix> _a_superlu{nullptr};
  std::unique_ptr<mfem::SuperLUSolver> _s_superlu{nullptr};
};
} // namespace Moose::MFEM

/**
 * Wrapper for Moose::MFEM::SuperLUSolver.
 */
class MFEMSuperLU : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMSuperLU(const InputParameters & parameters);

protected:
  void constructSolver(const InputParameters & parameters) override;

  /// Updates the solver with the bilinear form in case LOR solve is required
  void updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs) override;
};

#endif
