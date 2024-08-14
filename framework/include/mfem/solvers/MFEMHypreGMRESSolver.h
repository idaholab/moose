#pragma once
#include "MFEMSolverBase.h"
#include "mfem.hpp"
#include <memory>

/**
 * Wrapper for mfem::HypreGMRES solver.
 */
class MFEMHypreGMRESSolver : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMHypreGMRESSolver(const InputParameters &);

  /// Returns a shared pointer to the instance of the Solver derived-class.
  std::shared_ptr<mfem::Solver> getSolver() const override { return _solver; }

protected:
  void constructSolver(const InputParameters & parameters) override;

private:
  const MFEMPreconditionerBase & _preconditioner;
  std::shared_ptr<mfem::HypreGMRES> _solver{nullptr};
};
