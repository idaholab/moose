#pragma once
#include "MFEMIntegratedBC.h"
#include "MFEMVectorFunctionCoefficient.h"

class MFEMVectorNormalIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMVectorNormalIntegratedBC(const InputParameters & parameters);

  // Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
  // caller.
  virtual mfem::LinearFormIntegrator * createLinearFormIntegrator();

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBilinearFormIntegrator();

protected:
  MFEMVectorCoefficient * _vec_coef{nullptr};
};
