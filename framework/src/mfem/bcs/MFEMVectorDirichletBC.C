#ifdef MFEM_ENABLED

#include "MFEMVectorDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorDirichletBC);

MFEMVectorDirichletBC::MFEMVectorDirichletBC(const InputParameters & parameters)
  : MFEMVectorDirichletBCBase(parameters)
{
}

void
MFEMVectorDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh)
{
  mfem::Array<int> ess_bdrs(mesh.bdr_attributes.Max());
  ess_bdrs = getBoundaries();
  gridfunc.ProjectBdrCoefficient(_vec_coef, ess_bdrs);
}

#endif
