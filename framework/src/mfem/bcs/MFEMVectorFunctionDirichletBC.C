#include "MFEMVectorFunctionDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMVectorFunctionDirichletBC);

InputParameters
MFEMVectorFunctionDirichletBC::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<FunctionName>("function",
                                        "The values the components must take on the boundary.");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorFunctionDirichletBC::MFEMVectorFunctionDirichletBC(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _vec_coef(getMFEMProblem().getVectorFunctionCoefficient(getParam<FunctionName>("function"))),
    _boundary_apply_type{APPLY_TYPE::TANGENTIAL}
{
}

void
MFEMVectorFunctionDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  if (_vec_coef.get() == nullptr)
  {
    MFEM_ABORT("Boundary condition does not store valid coefficients to specify the "
               "components of the vector at the Dirichlet boundary.");
  }

  switch (_boundary_apply_type)
  {
    case STANDARD:
      gridfunc.ProjectBdrCoefficient(*_vec_coef, ess_bdrs);
      break;
    case NORMAL:
      gridfunc.ProjectBdrCoefficientNormal(*_vec_coef, ess_bdrs);
      break;
    case TANGENTIAL:
      gridfunc.ProjectBdrCoefficientTangent(*_vec_coef, ess_bdrs);
  }
}
