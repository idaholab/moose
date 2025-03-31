#include "MFEMVectorFunctorTangentialDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMVectorFunctorTangentialDirichletBC);

MFEMVectorFunctorTangentialDirichletBC::MFEMVectorFunctorTangentialDirichletBC(
    const InputParameters & parameters)
  : MFEMVectorFunctorDirichletBCBase(parameters)
{
}

void
MFEMVectorFunctorTangentialDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficientTangent(_vec_coef, ess_bdrs);
}
