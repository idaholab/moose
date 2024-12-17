#ifdef MFEM_ENABLED

#pragma once
#include "MFEMIntegratedBC.h"

class MFEMVectorFunctorBoundaryIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMVectorFunctorBoundaryIntegratedBC(const InputParameters & parameters);

  // Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
  // caller.
  virtual mfem::LinearFormIntegrator * createLinearFormIntegrator();

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBilinearFormIntegrator();

protected:
  mfem::VectorCoefficient & _vec_coef;
};

#endif
