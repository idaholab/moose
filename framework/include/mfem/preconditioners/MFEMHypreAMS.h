#pragma once
#include "MFEMPreconditionerBase.h"
#include "MFEMFESpace.h"

/**
 * Wrapper for mfem::HypreGMRES solver.
 */
class MFEMHypreAMS : public MFEMPreconditionerBase
{
public:
  static InputParameters validParams();

  MFEMHypreAMS(const InputParameters &);

  /// Returns a shared pointer to the instance of the Solver derived-class.
  std::shared_ptr<mfem::Solver> getPreconditioner() const override { return _preconditioner; }

protected:
  void constructPreconditioner(const InputParameters & parameters) override;

private:
  const MFEMFESpace & _mfem_fespace;
  std::shared_ptr<mfem::HypreAMS> _preconditioner{nullptr};
};
