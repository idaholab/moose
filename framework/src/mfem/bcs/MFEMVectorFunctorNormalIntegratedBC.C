#ifdef MFEM_ENABLED

#include "MFEMVectorFunctorNormalIntegratedBC.h"

registerMooseObject("MooseApp", MFEMVectorFunctorNormalIntegratedBC);

InputParameters
MFEMVectorFunctorNormalIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addRequiredParam<MFEMVectorCoefficientName>(
      "vector_functor",
      "Vector functor whose normal component will be used in the integrated BC. A functor is any "
      "of the following: a variable, an MFEM material property, a function, or a post-processor.");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctorNormalIntegratedBC::MFEMVectorFunctorNormalIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _vec_coef_name(getParam<MFEMVectorCoefficientName>("vector_functor")),
    _vec_coef(getVectorCoefficient(_vec_coef_name))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMVectorFunctorNormalIntegratedBC::createLFIntegrator()
{
  return new mfem::BoundaryNormalLFIntegrator(_vec_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMVectorFunctorNormalIntegratedBC::createBFIntegrator()
{
  return nullptr;
}

#endif
