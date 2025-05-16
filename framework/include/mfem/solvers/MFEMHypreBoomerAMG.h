#ifdef MFEM_ENABLED

#pragma once
#include "MFEMSolverBase.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"
#include <memory>

/**
 * Wrapper for mfem::HypreBoomerAMG solver.
 */
class MFEMHypreBoomerAMG : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMHypreBoomerAMG(const InputParameters &);

  /// Updates the solver with the bilinear form in case LOR solve is required
  void updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs) override;

protected:
  void constructSolver(const InputParameters & parameters) override;

private:
  std::shared_ptr<mfem::ParFiniteElementSpace> _mfem_fespace{nullptr};
  mfem::real_t _strength_threshold;
};

#endif
