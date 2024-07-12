#pragma once
#include "MFEMSolverBase.h"
#include "mfem.hpp"
#include <memory>

/**
 * Wrapper for mfem::HypreFGMRES solver.
 */
class MFEMHypreFGMRESSolver : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMHypreFGMRESSolver(const InputParameters & parameters);

  std::shared_ptr<mfem::Solver> getSolver() const override { return _solver; }

protected:
  void constructSolver(const InputParameters & parameters) override;

private:
  std::shared_ptr<mfem::HypreFGMRES> _solver{nullptr};
};
