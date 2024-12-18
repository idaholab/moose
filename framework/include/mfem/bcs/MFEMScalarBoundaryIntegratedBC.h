#pragma once
#include "MFEMIntegratedBC.h"

class MFEMScalarBoundaryIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMScalarBoundaryIntegratedBC(const InputParameters & parameters);

  // Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
  // caller.
  virtual mfem::LinearFormIntegrator * createLinearFormIntegrator();

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBilinearFormIntegrator();

protected:
  std::string _coef_name;
  mfem::Coefficient & _coef;
};
