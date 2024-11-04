#include "MFEMVectorBoundaryIntegratedBC.h"

registerMooseObject("PlatypusApp", MFEMVectorBoundaryIntegratedBC);

InputParameters
MFEMVectorBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addRequiredParam<std::string>(
      "vector_coefficient", "The vector MFEM coefficient which will be used in the integrated BC");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorBoundaryIntegratedBC::MFEMVectorBoundaryIntegratedBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _vec_coef_name(getParam<std::string>("vector_coefficient")),
    _vec_coef(const_cast<MFEMVectorCoefficient *>(
        &getUserObject<MFEMVectorCoefficient>("vector_coefficient")))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMVectorBoundaryIntegratedBC::createLinearFormIntegrator()
{
  return new mfem::VectorBoundaryLFIntegrator(*_vec_coef->getVectorCoefficient());
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMVectorBoundaryIntegratedBC::createBilinearFormIntegrator()
{
  return nullptr;
}
