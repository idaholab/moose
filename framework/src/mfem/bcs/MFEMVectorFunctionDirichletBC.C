#ifdef MFEM_ENABLED

#include "MFEMVectorFunctionDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorFunctionDirichletBC);

MFEMVectorFunctionDirichletBC::MFEMVectorFunctionDirichletBC(const InputParameters & parameters)
  : MFEMVectorFunctionDirichletBCBase(parameters)
{
}

void
MFEMVectorFunctionDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficient(*_vec_coef, ess_bdrs);
}

#endif
