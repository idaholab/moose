#include "MFEMVectorNormalDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMVectorNormalDirichletBC);

InputParameters
MFEMVectorNormalDirichletBC::validParams()
{
  InputParameters params = MFEMVectorDirichletBCBase::validParams();
  params.addClassDescription(
      "Applies a Dirichlet condition to the normal components of a vector variable.");
  return params;
}

MFEMVectorNormalDirichletBC::MFEMVectorNormalDirichletBC(const InputParameters & parameters)
  : MFEMVectorDirichletBCBase(parameters)
{
}

void
MFEMVectorNormalDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficientNormal(*_vec_coef, ess_bdrs);
}
