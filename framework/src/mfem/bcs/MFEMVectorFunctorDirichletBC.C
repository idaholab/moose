#ifdef MFEM_ENABLED

#include "MFEMVectorFunctorDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorFunctorDirichletBC);

MFEMVectorFunctorDirichletBC::MFEMVectorFunctorDirichletBC(const InputParameters & parameters)
  : MFEMVectorFunctorDirichletBCBase(parameters)
{
}

void
MFEMVectorFunctorDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh)
{
  mfem::Array<int> ess_bdrs(mesh.bdr_attributes.Max());
  ess_bdrs = getBoundaries();
  gridfunc.ProjectBdrCoefficient(_vec_coef, ess_bdrs);
}

#endif
