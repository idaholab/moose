#include "MFEMVectorFunctorNormalDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMVectorFunctorNormalDirichletBC);

InputParameters
MFEMVectorFunctorNormalDirichletBC::validParams()
{
  InputParameters params = MFEMVectorFunctorDirichletBCBase::validParams();
  params.addClassDescription(
      "Applies a Dirichlet condition to the normal components of a vector variable.");
  return params;
}

MFEMVectorFunctorNormalDirichletBC::MFEMVectorFunctorNormalDirichletBC(
    const InputParameters & parameters)
  : MFEMVectorFunctorDirichletBCBase(parameters)
{
}

void
MFEMVectorFunctorNormalDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficientNormal(_vec_coef, ess_bdrs);
}
