#ifdef MFEM_ENABLED

#include "MFEMVectorFunctionBoundaryIntegratedBC.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorFunctionBoundaryIntegratedBC);

InputParameters
MFEMVectorFunctionBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addRequiredParam<std::string>("vector_coefficient",
                                       "Vector coefficient used in the boundary integrator.");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctionBoundaryIntegratedBC::MFEMVectorFunctionBoundaryIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _vec_coef(getMFEMProblem().getProperties().getVectorProperty(
        getParam<std::string>("vector_coefficient")))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMVectorFunctionBoundaryIntegratedBC::createLFIntegrator()
{
  return new mfem::VectorBoundaryLFIntegrator(_vec_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMVectorFunctionBoundaryIntegratedBC::createBFIntegrator()
{
  return nullptr;
}

#endif
