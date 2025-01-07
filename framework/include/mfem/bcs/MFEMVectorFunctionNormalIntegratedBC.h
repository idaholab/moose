#pragma once
#include "MFEMIntegratedBC.h"

class MFEMVectorFunctionNormalIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMVectorFunctionNormalIntegratedBC(const InputParameters & parameters);

  // Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
  // caller.
  virtual mfem::LinearFormIntegrator * createLinearFormIntegrator();

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBilinearFormIntegrator();

protected:
  std::shared_ptr<mfem::VectorFunctionCoefficient> _vec_coef;
};
