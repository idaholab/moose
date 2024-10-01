#pragma once
#include "MFEMSolverBase.h"
#include "mfem.hpp"
#include <memory>

/**
 * Wrapper for mfem::OperatorJacobiSmoother solver.
 */
class MFEMOperatorJacobiSmoother : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMOperatorJacobiSmoother(const InputParameters & parameters);

  /// Returns a shared pointer to the instance of the Solver derived-class.
  std::shared_ptr<mfem::Solver> getSolver() const override { return _preconditioner; }

protected:
  void constructSolver(const InputParameters & parameters) override;

private:
  std::shared_ptr<mfem::OperatorJacobiSmoother> _preconditioner{nullptr};
};
