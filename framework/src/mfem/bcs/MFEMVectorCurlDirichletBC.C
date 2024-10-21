#include "MFEMVectorCurlDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMVectorCurlDirichletBC);

MFEMVectorCurlDirichletBC::MFEMVectorCurlDirichletBC(const InputParameters & parameters)
  : MFEMVectorDirichletBCBase(parameters)
{
}

void
MFEMVectorCurlDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficientTangent(*_vec_coef->getVectorCoefficient(), ess_bdrs);
}
