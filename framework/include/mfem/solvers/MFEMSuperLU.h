#pragma once
#include "MFEMSolverBase.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"
#include <memory>

namespace platypus
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
} // namespace platypus

/**
 * Wrapper for mfem::mfem::SuperLUSolver.
 */
class MFEMSuperLU : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMSuperLU(const InputParameters & parameters);

  /// Returns a shared pointer to the instance of the Solver derived-class.
  std::shared_ptr<mfem::Solver> getSolver() const override { return _solver; }

protected:
  void constructSolver(const InputParameters & parameters) override;

private:
  std::shared_ptr<platypus::SuperLUSolver> _solver{nullptr};
};
