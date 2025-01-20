#ifdef MFEM_ENABLED

#include "MFEMScalarFunctorBoundaryIntegratedBC.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMScalarFunctorBoundaryIntegratedBC);

InputParameters
MFEMScalarFunctorBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(f, v)_\\Omega$ "
                             "arising from the weak form of the forcing term $f$.");
  params.addRequiredParam<platypus::MFEMScalarCoefficientName>(
      "coefficient", "The scalar MFEM coefficient which will be used in the integrated BC.");
  return params;
}

MFEMScalarFunctorBoundaryIntegratedBC::MFEMScalarFunctorBoundaryIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _coef_name(getParam<platypus::MFEMScalarCoefficientName>("coefficient")),
    _coef(getScalarProperty(_coef_name))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMScalarFunctorBoundaryIntegratedBC::createLinearFormIntegrator()
{
  return new mfem::BoundaryLFIntegrator(_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMScalarFunctorBoundaryIntegratedBC::createBilinearFormIntegrator()
{
  return nullptr;
}

#endif
