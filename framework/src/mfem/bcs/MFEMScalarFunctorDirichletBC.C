#ifdef MFEM_ENABLED

#include "MFEMScalarFunctorDirichletBC.h"

registerMooseObject("MooseApp", MFEMScalarFunctorDirichletBC);

InputParameters
MFEMScalarFunctorDirichletBC::validParams()
{
  InputParameters params = MFEMEssentialBC::validParams();
  params.addRequiredParam<MFEMScalarCoefficientName>(
      "functor",
      "The functor setting the values on the essential boundary. A functor is any of the "
      "following: a variable, an MFEM material property, a function, or a post-processor.");
  return params;
}

MFEMScalarFunctorDirichletBC::MFEMScalarFunctorDirichletBC(const InputParameters & parameters)
  : MFEMEssentialBC(parameters),
    _coef_name(getParam<MFEMScalarCoefficientName>("functor")),
    _coef(getScalarCoefficient(_coef_name))
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
