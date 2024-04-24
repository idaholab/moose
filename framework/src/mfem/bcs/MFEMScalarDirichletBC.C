#include "MFEMScalarDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMScalarDirichletBC);

InputParameters
MFEMScalarDirichletBC::validParams()
{
  InputParameters params = MFEMBoundaryCondition::validParams();
  params.addRequiredParam<UserObjectName>(
      "coefficient", "The scalar MFEM coefficient to use in the Dirichlet condition");
  return params;
}

MFEMScalarDirichletBC::MFEMScalarDirichletBC(const InputParameters & parameters)
  : MFEMBoundaryCondition(parameters),
    _coef(const_cast<MFEMCoefficient *>(&getUserObject<MFEMCoefficient>("coefficient")))
{
  _boundary_condition = std::make_shared<hephaestus::ScalarDirichletBC>(
      getParam<std::string>("variable"), bdr_attr, _coef->getCoefficient().get());
}
