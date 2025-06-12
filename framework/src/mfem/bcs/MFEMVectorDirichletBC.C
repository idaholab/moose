#ifdef MFEM_ENABLED

#include "MFEMVectorDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorDirichletBC);

MFEMVectorDirichletBC::MFEMVectorDirichletBC(const InputParameters & parameters)
  : MFEMVectorDirichletBCBase(parameters)
{
}

void
MFEMVectorDirichletBC::ApplyBC(mfem::GridFunction & gridfunc)
{
  gridfunc.ProjectBdrCoefficient(_vec_coef, getBoundaries());
}

#endif
