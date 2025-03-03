#ifdef MFEM_ENABLED

#include "MFEMVectorFunctionTangentialDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorFunctionTangentialDirichletBC);

MFEMVectorFunctionTangentialDirichletBC::MFEMVectorFunctionTangentialDirichletBC(
    const InputParameters & parameters)
  : MFEMVectorFunctionDirichletBCBase(parameters)
{
}

void
MFEMVectorFunctionTangentialDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh)
{
  mfem::Array<int> ess_bdrs(mesh.bdr_attributes.Max());
  ess_bdrs = GetMarkers(mesh);
  gridfunc.ProjectBdrCoefficientTangent(*_vec_coef, ess_bdrs);
}

#endif
