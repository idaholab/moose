#ifdef MFEM_ENABLED

#pragma once
#include "MFEMIntegratedBC.h"

class MFEMVectorBoundaryIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMVectorBoundaryIntegratedBC(const InputParameters & parameters);

  // Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
  // caller.
  virtual mfem::LinearFormIntegrator * createLFIntegrator();

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBFIntegrator();

protected:
  std::vector<Real> _vec_value;
  mfem::VectorCoefficient & _vec_coef;
};

#endif
