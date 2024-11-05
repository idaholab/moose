#include "MFEMVectorFunctionBoundaryIntegratedBC.h"

registerMooseObject("PlatypusApp", MFEMVectorFunctionBoundaryIntegratedBC);

InputParameters
MFEMVectorFunctionBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addRequiredParam<FunctionName>("function",
                                        "The values the components must take on the boundary.");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctionBoundaryIntegratedBC::MFEMVectorFunctionBoundaryIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _vec_coef(getMFEMProblem().getVectorFunctionCoefficient(getParam<FunctionName>("function")))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMVectorFunctionBoundaryIntegratedBC::createLinearFormIntegrator()
{
  return new mfem::VectorBoundaryLFIntegrator(*_vec_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMVectorFunctionBoundaryIntegratedBC::createBilinearFormIntegrator()
{
  return nullptr;
}
