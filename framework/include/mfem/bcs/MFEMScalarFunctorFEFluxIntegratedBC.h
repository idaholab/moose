#ifdef MFEM_ENABLED

#pragma once
#include "MFEMIntegratedBC.h"

class MFEMScalarFunctorFEFluxIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMScalarFunctorFEFluxIntegratedBC(const InputParameters & parameters);

  // Create MFEM integrator to apply to the RHS of the weak form. Ownership managed by the caller.
  virtual mfem::LinearFormIntegrator * createLFIntegrator();

  // Create MFEM integrator to apply to the LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBFIntegrator();

protected:
  mfem::Coefficient & _coef;
};

#endif
