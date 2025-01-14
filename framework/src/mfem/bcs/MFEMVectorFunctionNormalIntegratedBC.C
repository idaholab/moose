#include "MFEMVectorFunctionNormalIntegratedBC.h"

registerMooseObject("PlatypusApp", MFEMVectorFunctionNormalIntegratedBC);

InputParameters
MFEMVectorFunctionNormalIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addRequiredParam<FunctionName>(
      "function", "The vector function whose normal component will be used in the integrated BC");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctionNormalIntegratedBC::MFEMVectorFunctionNormalIntegratedBC(
    const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _vec_coef(getMFEMProblem().getVectorFunctionCoefficient(getParam<FunctionName>("function")))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMVectorFunctionNormalIntegratedBC::createLinearFormIntegrator()
{
  return new mfem::BoundaryNormalLFIntegrator(*_vec_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMVectorFunctionNormalIntegratedBC::createBilinearFormIntegrator()
{
  return nullptr;
}
