#ifdef MFEM_ENABLED

#pragma once
#include "MFEMIntegratedBC.h"

/*
(h (T-T_inf), T')
*/
class MFEMConvectiveHeatFluxBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMConvectiveHeatFluxBC(const InputParameters & parameters);

  // Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
  // caller.
  virtual mfem::LinearFormIntegrator * createLFIntegrator() override;

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;

protected:
  mfem::Coefficient & _heat_transfer_coef;
  mfem::Coefficient & _T_inf_coef;
  mfem::ProductCoefficient & _external_heat_flux_coef;
};

#endif
