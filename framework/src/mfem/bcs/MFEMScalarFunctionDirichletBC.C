#ifdef MFEM_ENABLED

#include "MFEMScalarFunctionDirichletBC.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMScalarFunctionDirichletBC);

InputParameters
MFEMScalarFunctionDirichletBC::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  return params;
}

MFEMScalarFunctionDirichletBC::MFEMScalarFunctionDirichletBC(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _coef_name(getParam<FunctionName>("function")),
    _coef(getMFEMProblem().getProperties().getScalarProperty(_coef_name))
{
}

void
MFEMScalarFunctionDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh)
{
  mfem::Array<int> ess_bdrs(mesh.bdr_attributes.Max());
  ess_bdrs = getBoundaries();
  gridfunc.ProjectBdrCoefficient(_coef, ess_bdrs);
}

#endif
