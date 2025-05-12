#ifdef MFEM_ENABLED

#include "MFEMVectorFunctorNormalDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorFunctorNormalDirichletBC);

InputParameters
MFEMVectorFunctorNormalDirichletBC::validParams()
{
  InputParameters params = MFEMVectorFunctorDirichletBCBase::validParams();
  params.addClassDescription(
      "Applies a Dirichlet condition to the normal components of a vector variable.");
  return params;
}

MFEMVectorFunctorNormalDirichletBC::MFEMVectorFunctorNormalDirichletBC(
    const InputParameters & parameters)
  : MFEMVectorFunctorDirichletBCBase(parameters)
{
}

void
MFEMVectorFunctorNormalDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh)
{
  mfem::Array<int> ess_bdrs(mesh.bdr_attributes.Max());
  ess_bdrs = getBoundaries();
  gridfunc.ProjectBdrCoefficientNormal(_vec_coef, ess_bdrs);
}

#endif
