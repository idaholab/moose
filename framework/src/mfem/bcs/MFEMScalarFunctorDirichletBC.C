#include "MFEMScalarFunctorDirichletBC.h"

registerMooseObject("MooseApp", MFEMScalarFunctorDirichletBC);

InputParameters
MFEMScalarFunctorDirichletBC::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<platypus::MFEMScalarCoefficientName>(
      "coefficient", "The coefficient setting the values on the essential boundary.");
  return params;
}

MFEMScalarFunctorDirichletBC::MFEMScalarFunctorDirichletBC(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _coef_name(getParam<platypus::MFEMScalarCoefficientName>("coefficient")),
    _coef(getScalarProperty(_coef_name))
{
}

void
MFEMScalarFunctorDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_)
{
  mfem::Array<int> ess_bdrs(mesh_->bdr_attributes.Max());
  ess_bdrs = GetMarkers(*mesh_);
  gridfunc.ProjectBdrCoefficient(_coef, ess_bdrs);
}
