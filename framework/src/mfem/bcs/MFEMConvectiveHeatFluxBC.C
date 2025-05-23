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
  params.addRequiredParam<MFEMScalarCoefficientName>(
      "T_infinity",
      "Name of a coefficient specifying the far-field temperature. A coefficient can be any of the "
      "following: "
      "a variable, an MFEM material property, a function, or a post-processor.");
  params.addRequiredParam<MFEMScalarCoefficientName>(
      "heat_transfer_coefficient",
      "Name of the coefficient specifying the heat transfer coefficient. A coefficient can be any "
      "of the "
      "following: a variable, an MFEM material property, a function, or a post-processor.");
  return params;
}

MFEMConvectiveHeatFluxBC::MFEMConvectiveHeatFluxBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _heat_transfer_coef(
        getScalarCoefficient(getParam<MFEMScalarCoefficientName>("heat_transfer_coefficient"))),
    _T_inf_coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("T_infinity"))),
    _external_heat_flux_coef(
        getMFEMProblem().getCoefficients().declareScalar<mfem::ProductCoefficient>(
            "__ConvectiveHeatFluxBC_" + parameters.get<std::string>("_unique_name"),
            _heat_transfer_coef,
            _T_inf_coef))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMConvectiveHeatFluxBC::createLFIntegrator()
{
  return new mfem::BoundaryLFIntegrator(_external_heat_flux_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMConvectiveHeatFluxBC::createBFIntegrator()
{
  return new mfem::BoundaryMassIntegrator(_heat_transfer_coef);
}

#endif
