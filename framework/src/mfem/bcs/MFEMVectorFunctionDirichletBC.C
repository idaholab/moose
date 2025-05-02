#ifdef MFEM_ENABLED

#include "MFEMVectorFunctionDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorFunctionDirichletBC);

MFEMVectorFunctionDirichletBC::MFEMVectorFunctionDirichletBC(const InputParameters & parameters)
  : MFEMVectorFunctionDirichletBCBase(parameters)
{
}

void
MFEMVectorFunctionDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh)
{
  mfem::Array<int> ess_bdrs(mesh.bdr_attributes.Max());
  ess_bdrs = getBoundaries();
  gridfunc.ProjectBdrCoefficient(*_vec_coef, ess_bdrs);
}

#endif
