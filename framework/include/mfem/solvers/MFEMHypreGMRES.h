#pragma once
#include "MFEMSolverBase.h"
#include "MFEMHypreBoomerAMG.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"
#include <memory>

/**
 * Wrapper for mfem::HypreGMRES solver.
 */
class MFEMHypreGMRES : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMHypreGMRES(const InputParameters &);

  /// Returns a shared pointer to the instance of the Solver derived-class.
  std::shared_ptr<mfem::Solver> getSolver() const override { return _solver; }

protected:
  void constructSolver(const InputParameters & parameters) override;

private:
  std::shared_ptr<mfem::Solver> _preconditioner{nullptr};
  std::shared_ptr<mfem::HypreGMRES> _solver{nullptr};
};
