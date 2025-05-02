#ifdef MFEM_ENABLED

#pragma once
#include "MFEMSolverBase.h"
#include "MFEMFESpace.h"

/**
 * Wrapper for mfem::HypreAMS solver.
 */
class MFEMHypreAMS : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMHypreAMS(const InputParameters &);

  /// Returns a shared pointer to the instance of the Solver derived-class.
  std::shared_ptr<mfem::Solver> getSolver() override { return _preconditioner; }

  // Updates the solver with the bilinear form in case LOR solve is required
  void updateSolver(mfem::ParBilinearForm &a, mfem::Array<int> &tdofs) override;

protected:
  void constructSolver(const InputParameters & parameters) override;

private:
  const MFEMFESpace & _mfem_fespace;
  std::shared_ptr<mfem::HypreAMS> _jacobian_preconditioner{nullptr};
  std::shared_ptr<mfem::Solver> _preconditioner{nullptr};
};

#endif
