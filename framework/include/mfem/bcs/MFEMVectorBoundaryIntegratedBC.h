#pragma once
#include "MFEMIntegratedBC.h"
#include "MFEMVectorFunctionCoefficient.h"

class MFEMVectorBoundaryIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMVectorBoundaryIntegratedBC(const InputParameters & parameters);

  // Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
  // caller.
  virtual mfem::LinearFormIntegrator * createLinearFormIntegrator();

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBilinearFormIntegrator();

protected:
  std::string _vec_coef_name;
  MFEMVectorCoefficient * _vec_coef;
};
