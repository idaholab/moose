#ifdef MFEM_ENABLED

#include "MFEMVectorFunctorBoundaryIntegratedBC.h"

registerMooseObject("PlatypusApp", MFEMVectorFunctorBoundaryIntegratedBC);

InputParameters
MFEMVectorFunctorBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addRequiredParam<MFEMVectorCoefficientName>(
      "vector_functor",
      "Vector functor used in the boundary integrator. A functor is any of the following: a "
      "variable, an MFEM material property, a function, or a post-processor.");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctorBoundaryIntegratedBC::MFEMVectorFunctorBoundaryIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _vec_coef(getVectorCoefficient(getParam<MFEMVectorCoefficientName>("vector_functor")))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMVectorFunctorBoundaryIntegratedBC::createLFIntegrator()
{
  return new mfem::VectorBoundaryLFIntegrator(_vec_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMVectorFunctorBoundaryIntegratedBC::createBFIntegrator()
{
  return nullptr;
}

#endif
