#pragma once
#include "MFEMSolverBase.h"
#include "MFEMFESpace.h"

/**
 * Wrapper for mfem::HypreADS solver.
 */
class MFEMHypreADS : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMHypreADS(const InputParameters &);

  /// Returns a shared pointer to the instance of the Solver derived-class.
  std::shared_ptr<mfem::Solver> getSolver() const override { return _preconditioner; }

protected:
  void constructSolver(const InputParameters & parameters) override;

private:
  const MFEMFESpace & _mfem_fespace;
  std::shared_ptr<mfem::HypreADS> _preconditioner{nullptr};
};
