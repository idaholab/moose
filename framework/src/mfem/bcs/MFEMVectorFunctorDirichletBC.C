#include "MFEMVectorFunctorDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMVectorFunctorDirichletBC);

MFEMVectorFunctorDirichletBC::MFEMVectorFunctorDirichletBC(const InputParameters & parameters)
  : MFEMVectorFunctorDirichletBCBase(parameters)
{
}

void
MFEMVectorFunctorDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficient(_vec_coef, ess_bdrs);
}
