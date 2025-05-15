#ifdef MFEM_ENABLED

#pragma once
#include "MFEMSolverBase.h"
#include "MFEMFESpace.h"

/**
 * Wrapper for mfem::HypreADS solver.
 */
class MFEMHypreADS : public MFEMSolverBase
{
public:
  static InputParameters validParams();

  MFEMHypreADS(const InputParameters &);

  // Updates the solver with the bilinear form in case LOR solve is required
  void updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs) override;

protected:
  void constructSolver(const InputParameters & parameters) override;

private:
  const MFEMFESpace & _mfem_fespace;
};

#endif
