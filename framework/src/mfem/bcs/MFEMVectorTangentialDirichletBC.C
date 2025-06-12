#ifdef MFEM_ENABLED

#include "MFEMVectorTangentialDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorTangentialDirichletBC);

MFEMVectorTangentialDirichletBC::MFEMVectorTangentialDirichletBC(const InputParameters & parameters)
  : MFEMVectorDirichletBCBase(parameters)
{
}

void
MFEMVectorTangentialDirichletBC::ApplyBC(mfem::GridFunction & gridfunc)
{
  gridfunc.ProjectBdrCoefficientTangent(_vec_coef, getBoundaries());
}

#endif
