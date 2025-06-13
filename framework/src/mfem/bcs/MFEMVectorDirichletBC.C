#ifdef MFEM_ENABLED

#include "MFEMVectorDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorDirichletBC);

InputParameters
MFEMVectorDirichletBC::validParams()
{
  InputParameters params = MFEMVectorDirichletBCBase::validParams();
  params.addClassDescription(
      "Applies a Dirichlet condition to all components of a vector variable.");
  return params;
}

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
