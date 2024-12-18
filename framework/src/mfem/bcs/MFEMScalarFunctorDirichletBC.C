#ifdef MFEM_ENABLED

#include "MFEMScalarFunctorDirichletBC.h"

registerMooseObject("MooseApp", MFEMScalarFunctorDirichletBC);

InputParameters
MFEMScalarFunctorDirichletBC::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<MFEMScalarCoefficientName>(
      "coefficient", "The coefficient setting the values on the essential boundary.");
  return params;
}

MFEMScalarFunctorDirichletBC::MFEMScalarFunctorDirichletBC(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _coef_name(getParam<MFEMScalarCoefficientName>("coefficient")),
    _coef(getScalarProperty(_coef_name))
{
}

void
MFEMScalarFunctorDirichletBC::ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh & mesh)
{
  mfem::Array<int> ess_bdrs(mesh.bdr_attributes.Max());
  ess_bdrs = getBoundaries();
  gridfunc.ProjectBdrCoefficient(_coef, ess_bdrs);
}

#endif
