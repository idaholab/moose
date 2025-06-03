#ifdef MFEM_ENABLED

#include "MFEMVectorFunctorDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorFunctorDirichletBC);

InputParameters
MFEMVectorFunctorDirichletBC::validParams()
{
  InputParameters params = MFEMVectorFunctorDirichletBCBase::validParams();
  params.addClassDescription(
      "Applies a Dirichlet condition to all components of a vector variable.");
  return params;
}

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
