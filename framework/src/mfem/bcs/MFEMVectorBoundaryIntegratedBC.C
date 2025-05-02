#ifdef MFEM_ENABLED

#include "MFEMVectorBoundaryIntegratedBC.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorBoundaryIntegratedBC);

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

MFEMVectorBoundaryIntegratedBC::MFEMVectorBoundaryIntegratedBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _vec_value(getParam<std::vector<Real>>("values")),
    _vec_coef(getMFEMProblem().getCoefficients().declareVector<mfem::VectorConstantCoefficient>(
        "__VectorBoundaryIntegratedBC_" + parameters.get<std::string>("_unique_name"),
        mfem::Vector(_vec_value.data(), _vec_value.size())))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMVectorBoundaryIntegratedBC::createLFIntegrator()
{
  return new mfem::VectorBoundaryLFIntegrator(_vec_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMVectorBoundaryIntegratedBC::createBFIntegrator()
{
  return nullptr;
}

#endif
