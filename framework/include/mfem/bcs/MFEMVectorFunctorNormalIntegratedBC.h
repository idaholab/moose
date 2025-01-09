#ifdef MFEM_ENABLED

#pragma once
#include "MFEMIntegratedBC.h"

class MFEMVectorFunctorNormalIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMVectorFunctorNormalIntegratedBC(const InputParameters & parameters);

  // Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
  // caller.
  virtual mfem::LinearFormIntegrator * createLinearFormIntegrator();

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBilinearFormIntegrator();

protected:
  const platypus::MFEMVectorCoefficientName & _vec_coef_name;
  mfem::VectorCoefficient & _vec_coef;
};

#endif
