#ifdef MFEM_ENABLED

#include "MFEMConvectiveHeatFluxBC.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMConvectiveHeatFluxBC);

InputParameters
MFEMConvectiveHeatFluxBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  // FIXME: Should these really be specified via properties? T_infinity in particular? Use functions
  // instead?
  params.addClassDescription(
      "Convective heat transfer boundary condition with temperature and heat "
      "transfer coefficent given by material properties to add to MFEM problems.");
  params.addRequiredParam<FunctionName>("T_infinity", "Function for far-field temperature");
  params.addRequiredParam<FunctionName>("heat_transfer_coefficient",
                                        "Function for heat transfer coefficient");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMConvectiveHeatFluxBC::MFEMConvectiveHeatFluxBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _heat_transfer_coef(getMFEMProblem().getProperties().getScalarProperty(
        getParam<FunctionName>("heat_transfer_coefficient"))),
    _T_inf_coef(
        getMFEMProblem().getProperties().getScalarProperty(getParam<FunctionName>("T_infinity"))),
    _external_heat_flux_coef(getMFEMProblem().makeScalarCoefficient<mfem::ProductCoefficient>(
        _heat_transfer_coef, _T_inf_coef))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMConvectiveHeatFluxBC::createLFIntegrator()
{
  return new mfem::BoundaryLFIntegrator(*_external_heat_flux_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMConvectiveHeatFluxBC::createBFIntegrator()
{
  return new mfem::BoundaryMassIntegrator(_heat_transfer_coef);
}

#endif
