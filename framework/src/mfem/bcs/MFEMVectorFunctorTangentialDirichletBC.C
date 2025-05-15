#ifdef MFEM_ENABLED

#include "MFEMVectorFunctorTangentialDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorFunctorTangentialDirichletBC);

MFEMVectorFunctorTangentialDirichletBC::MFEMVectorFunctorTangentialDirichletBC(
    const InputParameters & parameters)
  : MFEMVectorFunctorDirichletBCBase(parameters)
{
}

void
MFEMVectorFunctorTangentialDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh)
{
  mfem::Array<int> ess_bdrs(mesh.bdr_attributes.Max());
  ess_bdrs = getBoundaries();
  gridfunc.ProjectBdrCoefficientTangent(_vec_coef, ess_bdrs);
}

#endif
