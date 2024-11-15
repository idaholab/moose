#include "MFEMScalarBoundaryIntegratedBC.h"

registerMooseObject("PlatypusApp", MFEMScalarBoundaryIntegratedBC);

InputParameters
MFEMScalarBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addRequiredParam<std::string>(
      "coefficient", "The scalar MFEM coefficient which will be used in the integrated BC.");
  return params;
}

MFEMScalarBoundaryIntegratedBC::MFEMScalarBoundaryIntegratedBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    _coef(const_cast<MFEMCoefficient *>(&getUserObject<MFEMCoefficient>("coefficient")))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMScalarBoundaryIntegratedBC::createLinearFormIntegrator()
{
  return new mfem::BoundaryLFIntegrator(*_coef->getCoefficient());
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMScalarBoundaryIntegratedBC::createBilinearFormIntegrator()
{
  return nullptr;
}
