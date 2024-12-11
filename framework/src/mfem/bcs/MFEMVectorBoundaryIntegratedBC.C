#include "MFEMVectorBoundaryIntegratedBC.h"

registerMooseObject("PlatypusApp", MFEMVectorBoundaryIntegratedBC);

InputParameters
MFEMVectorBoundaryIntegratedBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription("Adds the boundary integrator to an MFEM problem for the linear form "
                             "$(\\vec f, \\vec v)_{\\partial\\Omega}$");
  params.addRequiredParam<std::vector<Real>>(
      "values", "The vector whose components will be used in the integrated BC");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorBoundaryIntegratedBC::MFEMVectorBoundaryIntegratedBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _vec_value(getParam<std::vector<Real>>("values")),
    _vec_coef(getMFEMProblem().makeVectorCoefficient<mfem::VectorConstantCoefficient>(
        mfem::Vector(_vec_value.data(), _vec_value.size())))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMVectorBoundaryIntegratedBC::createLinearFormIntegrator()
{
  return new mfem::VectorBoundaryLFIntegrator(*_vec_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMVectorBoundaryIntegratedBC::createBilinearFormIntegrator()
{
  return nullptr;
}
