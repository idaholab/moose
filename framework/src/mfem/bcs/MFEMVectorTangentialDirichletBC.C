#include "MFEMVectorTangentialDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMVectorTangentialDirichletBC);

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
