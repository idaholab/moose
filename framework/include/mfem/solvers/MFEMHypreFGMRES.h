#ifdef MFEM_ENABLED

#pragma once
#include "MFEMSolverBase.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"
#include <memory>

/**
 * Wrapper for mfem::HypreFGMRES solver.
 */
class MFEMHypreFGMRES : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMHypreFGMRES(const InputParameters & parameters);

  std::shared_ptr<mfem::Solver> getSolver() const override { return _solver; }

protected:
  void constructSolver(const InputParameters & parameters) override;

private:
  std::shared_ptr<mfem::Solver> _preconditioner{nullptr};
  std::shared_ptr<mfem::HypreFGMRES> _solver{nullptr};
};

#endif
