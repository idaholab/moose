#ifdef MFEM_ENABLED

#pragma once
#include "MFEMSolverBase.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"
#include <memory>

namespace Moose::MFEM
{

/**
 * Wrapper for mfem::SuperLU solver that creates a SuperLURowLocMatrix from the operator
 * when set.
 */
class SuperLUSolver : public mfem::SuperLUSolver
{
public:
  SuperLUSolver(MPI_Comm comm, int npdep = 1) : mfem::SuperLUSolver(comm, npdep){};
  void SetOperator(const mfem::Operator & op) override
  {
    _a_superlu = std::make_unique<mfem::SuperLURowLocMatrix>(op);
    mfem::SuperLUSolver::SetOperator(*_a_superlu.get());
  }

private:
  std::unique_ptr<mfem::SuperLURowLocMatrix> _a_superlu{nullptr};
};
} // namespace Moose::MFEM

/**
 * Wrapper for mfem::mfem::SuperLUSolver.
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
