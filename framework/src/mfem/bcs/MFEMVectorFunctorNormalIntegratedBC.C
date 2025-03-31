#ifdef MFEM_ENABLED

#include "MFEMVectorFunctorNormalIntegratedBC.h"

registerMooseObject("MooseApp", MFEMVectorFunctorNormalIntegratedBC);

InputParameters
MFEMVectorFunctorNormalIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addRequiredParam<std::string>(
      "vector_coefficient",
      "The vector function whose normal component will be used in the integrated BC");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctorNormalIntegratedBC::MFEMVectorFunctorNormalIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _vec_coef(getMFEMProblem().getProperties().getVectorProperty(
        getParam<std::string>("vector_coefficient")))
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
