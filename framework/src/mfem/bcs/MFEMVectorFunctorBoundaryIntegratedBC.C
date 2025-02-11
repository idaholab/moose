#include "MFEMVectorFunctorBoundaryIntegratedBC.h"

registerMooseObject("MooseApp", MFEMVectorFunctorBoundaryIntegratedBC);

InputParameters
MFEMVectorFunctorBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addRequiredParam<platypus::MFEMVectorCoefficientName>(
      "vector_coefficient", "Vector coefficient used in the boundary integrator.");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctorBoundaryIntegratedBC::MFEMVectorFunctorBoundaryIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _vec_coef(
        getVectorProperty(getParam<platypus::MFEMVectorCoefficientName>("vector_coefficient")))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMVectorFunctorBoundaryIntegratedBC::createLinearFormIntegrator()
{
  return new mfem::VectorBoundaryLFIntegrator(_vec_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMVectorFunctorBoundaryIntegratedBC::createBilinearFormIntegrator()
{
  return nullptr;
}
