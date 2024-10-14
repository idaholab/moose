#include "MFEMConvectiveHeatFluxBC.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMConvectiveHeatFluxBC);

InputParameters
MFEMConvectiveHeatFluxBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription(
      "Convective heat transfer boundary condition with temperature and heat "
      "transfer coefficent given by material properties to add to MFEM problems.");
  params.addRequiredParam<std::string>("T_infinity", "Material property for far-field temperature");
  params.addRequiredParam<std::string>("heat_transfer_coefficient",
                                       "Material property for heat transfer coefficient");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMConvectiveHeatFluxBC::MFEMConvectiveHeatFluxBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _heat_transfer_coef_name(getParam<std::string>("heat_transfer_coefficient")),
    _T_inf_coef_name(getParam<std::string>("T_infinity")),
    _heat_transfer_coef(
        getMFEMProblem().getProperties().getScalarProperty(_heat_transfer_coef_name)),
    _T_inf_coef(getMFEMProblem().getProperties().getScalarProperty(_T_inf_coef_name)),
    _external_heat_flux_coef(_heat_transfer_coef, _T_inf_coef)
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMConvectiveHeatFluxBC::createLinearFormIntegrator()
{
  return new mfem::BoundaryLFIntegrator(_external_heat_flux_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMConvectiveHeatFluxBC::createBilinearFormIntegrator()
{
  return new mfem::BoundaryMassIntegrator(_heat_transfer_coef);
}