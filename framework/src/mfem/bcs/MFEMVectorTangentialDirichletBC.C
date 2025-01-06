#ifdef MFEM_ENABLED

#include "MFEMVectorTangentialDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorTangentialDirichletBC);

InputParameters
MFEMVectorTangentialDirichletBC::validParams()
{
  InputParameters params = MFEMVectorDirichletBCBase::validParams();
  params.addClassDescription(
      "Applies a Dirichlet condition to the tangential components of a vector variable.");
  return params;
}

MFEMVectorTangentialDirichletBC::MFEMVectorTangentialDirichletBC(const InputParameters & parameters)
  : MFEMVectorDirichletBCBase(parameters)
{
}

void
MFEMVectorTangentialDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficientTangent(*_vec_coef, ess_bdrs);
}

#endif
