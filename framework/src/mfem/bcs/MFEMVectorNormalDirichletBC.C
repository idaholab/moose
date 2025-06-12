#ifdef MFEM_ENABLED

#include "MFEMVectorNormalDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorNormalDirichletBC);

InputParameters
MFEMVectorNormalDirichletBC::validParams()
{
  InputParameters params = MFEMVectorDirichletBCBase::validParams();
  params.addClassDescription(
      "Applies a Dirichlet condition to the normal components of a vector variable.");
  return params;
}

MFEMVectorNormalDirichletBC::MFEMVectorNormalDirichletBC(const InputParameters & parameters)
  : MFEMVectorDirichletBCBase(parameters)
{
}

void
MFEMVectorNormalDirichletBC::ApplyBC(mfem::GridFunction & gridfunc)
{
  gridfunc.ProjectBdrCoefficientNormal(_vec_coef, getBoundaries());
}

#endif
