#include "MFEMScalarDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMScalarDirichletBC);

InputParameters
MFEMScalarDirichletBC::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<UserObjectName>(
      "coefficient", "The scalar MFEM coefficient to use in the Dirichlet condition");
  return params;
}

MFEMScalarDirichletBC::MFEMScalarDirichletBC(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _coef(const_cast<MFEMCoefficient *>(&getUserObject<MFEMCoefficient>("coefficient")))
{
}

void
MFEMScalarDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficient(*_coef->getCoefficient(), ess_bdrs);
}
