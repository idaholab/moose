#ifdef MFEM_ENABLED

#include "MFEMVectorTangentialDirichletBC.h"

registerMooseObject("MooseApp", MFEMVectorTangentialDirichletBC);

InputParameters
MFEMVectorTangentialDirichletBC::validParams()
{
  InputParameters params = MFEMVectorDirichletBCBase::validParams();
  params.addClassDescription(
      "Applies a Dirichlet condition to the tangential components of a vector variable.");
  return params;
}

MFEMVectorTangentialDirichletBC::MFEMVectorTangentialDirichletBC(const InputParameters & parameters)
  : MFEMVectorDirichletBCBase(parameters)
{
}

void
MFEMVectorTangentialDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh)
{
  mfem::Array<int> ess_bdrs(mesh.bdr_attributes.Max());
  ess_bdrs = GetMarkers(mesh);
  gridfunc.ProjectBdrCoefficientTangent(*_vec_coef, ess_bdrs);
}

#endif
