#ifdef MFEM_ENABLED

#include "MFEMScalarFunctorFEFluxIntegratedBC.h"

registerMooseObject("MooseApp", MFEMScalarFunctorFEFluxIntegratedBC);

InputParameters
MFEMScalarFunctorFEFluxIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addRequiredParam<MFEMScalarCoefficientName>(
      "coefficient",
      "The coefficient which will be used in the integrated BC. A coefficient can be be any of "
      "the following: a variable, an MFEM material property, a function, or a post-processor.");
  return params;
}

MFEMScalarFunctorFEFluxIntegratedBC::MFEMScalarFunctorFEFluxIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient")))
{
}

// Create MFEM integrator to apply to the RHS of the weak form. Ownership managed by the caller.
mfem::LinearFormIntegrator *
MFEMScalarFunctorFEFluxIntegratedBC::createLFIntegrator()
{
  return new mfem::VectorFEBoundaryFluxLFIntegrator(_coef);
}

// Create MFEM integrator to apply to the LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMScalarFunctorFEFluxIntegratedBC::createBFIntegrator()
{
  return nullptr;
}

#endif
