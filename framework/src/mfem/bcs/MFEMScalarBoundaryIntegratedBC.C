#ifdef MFEM_ENABLED

#include "MFEMScalarBoundaryIntegratedBC.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMScalarBoundaryIntegratedBC);

InputParameters
MFEMScalarBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(f, v)_\\Omega$ "
                             "arising from the weak form of the forcing term $f$.");
  params.addRequiredParam<std::string>(
      "coefficient", "The scalar MFEM coefficient which will be used in the integrated BC.");
  return params;
}

MFEMScalarBoundaryIntegratedBC::MFEMScalarBoundaryIntegratedBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    _coef(getMFEMProblem().getProperties().getScalarProperty(_coef_name))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMScalarBoundaryIntegratedBC::createLinearFormIntegrator()
{
  return new mfem::BoundaryLFIntegrator(_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMScalarBoundaryIntegratedBC::createBilinearFormIntegrator()
{
  return nullptr;
}

#endif
