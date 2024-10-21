#include "MFEMVectorDivDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMVectorDivDirichletBC);

MFEMVectorDivDirichletBC::MFEMVectorDivDirichletBC(const InputParameters & parameters)
  : MFEMVectorDirichletBCBase(parameters)
{
}

void
MFEMVectorDivDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficientNormal(*_vec_coef->getVectorCoefficient(), ess_bdrs);
}
