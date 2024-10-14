#pragma once
#include "MFEMIntegratedBC.h"
#include "MFEMCoefficient.h"

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
  virtual mfem::LinearFormIntegrator * createLinearFormIntegrator();

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBilinearFormIntegrator();

protected:
  std::string _heat_transfer_coef_name;
  std::string _T_inf_coef_name;
  mfem::Coefficient & _heat_transfer_coef;
  mfem::Coefficient & _T_inf_coef;
  mfem::ProductCoefficient _external_heat_flux_coef;
};
