#ifdef MFEM_ENABLED

#pragma once
#include "MFEMSolverBase.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"
#include <memory>

/**
 * Wrapper for mfem::HyprePCG solver.
 */
class MFEMHyprePCG : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMHyprePCG(const InputParameters & parameters);

  /// Returns a shared pointer to the instance of the Solver derived-class.
  std::shared_ptr<mfem::Solver> getSolver() override { return _solver; }

  // Updates the solver with the bilinear form in case LOR solve is required
  void updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs) override;

protected:
  void constructSolver(const InputParameters & parameters) override;

private:
  std::shared_ptr<MFEMSolverBase> _preconditioner{nullptr};
  std::shared_ptr<mfem::HyprePCG> _jacobian_solver{nullptr};
  std::shared_ptr<mfem::Solver> _solver{nullptr};
};

#endif
