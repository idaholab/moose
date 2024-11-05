#include "MFEMScalarDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMScalarDirichletBC);

InputParameters
MFEMScalarDirichletBC::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<Real>("value", "Value of the BC.");
  return params;
}

MFEMScalarDirichletBC::MFEMScalarDirichletBC(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _coef(
        getMFEMProblem().makeScalarCoefficient<mfem::ConstantCoefficient>(getParam<Real>("value")))
{
}

void
MFEMScalarDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficient(*_coef, ess_bdrs);
}
