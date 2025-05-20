#ifdef MFEM_ENABLED

#pragma once
#include "MFEMIntegratedBC.h"

class MFEMScalarFunctorBoundaryIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMScalarFunctorBoundaryIntegratedBC(const InputParameters & parameters);

  // Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
  // caller.
  virtual mfem::LinearFormIntegrator * createLFIntegrator();

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBFIntegrator();

protected:
  const MFEMScalarCoefficientName & _coef_name;
  mfem::Coefficient & _coef;
};

#endif
