#include "MFEMVectorFunctionTangentialDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMVectorFunctionTangentialDirichletBC);

MFEMVectorFunctionTangentialDirichletBC::MFEMVectorFunctionTangentialDirichletBC(
    const InputParameters & parameters)
  : MFEMVectorFunctionDirichletBCBase(parameters)
{
}

void
MFEMVectorFunctionTangentialDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficientTangent(*_vec_coef, ess_bdrs);
}
