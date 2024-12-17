#include "MFEMScalarFunctionDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMScalarFunctionDirichletBC);

InputParameters
MFEMScalarFunctionDirichletBC::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  return params;
}

MFEMScalarFunctionDirichletBC::MFEMScalarFunctionDirichletBC(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _coef(getMFEMProblem().getScalarFunctionCoefficient(getParam<FunctionName>("function")))
{
}

void
MFEMScalarFunctionDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficient(*_coef, ess_bdrs);
}
