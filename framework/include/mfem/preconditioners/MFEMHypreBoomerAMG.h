#pragma once
#include "MFEMPreconditionerBase.h"
#include "mfem.hpp"
#include <memory>

/**
 * Wrapper for mfem::HypreGMRES solver.
 */
class MFEMHypreBoomerAMG : public MFEMPreconditionerBase
{
public:
  static InputParameters validParams();

  MFEMHypreBoomerAMG(const InputParameters &);

  /// Returns a shared pointer to the instance of the Solver derived-class.
  std::shared_ptr<mfem::Solver> getPreconditioner() const override { return _preconditioner; }

protected:
  void constructPreconditioner(const InputParameters & parameters) override;

private:
  std::shared_ptr<mfem::HypreBoomerAMG> _preconditioner{nullptr};
};
