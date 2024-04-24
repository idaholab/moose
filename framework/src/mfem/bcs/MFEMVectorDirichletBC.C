#include "MFEMVectorDirichletBC.h"

registerMooseObject("PlatypusApp", MFEMVectorDirichletBC);

InputParameters
MFEMVectorDirichletBC::validParams()
{
  InputParameters params = MFEMBoundaryCondition::validParams();
  params.addRequiredParam<UserObjectName>(
      "vector_coefficient", "The vector MFEM coefficient to use in the Dirichlet condition");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorDirichletBC::MFEMVectorDirichletBC(const InputParameters & parameters)
  : MFEMBoundaryCondition(parameters),
    _vec_coef(const_cast<MFEMVectorCoefficient *>(
        &getUserObject<MFEMVectorCoefficient>("vector_coefficient")))
{
  _boundary_condition = std::make_shared<hephaestus::VectorDirichletBC>(
      getParam<std::string>("variable"), bdr_attr, _vec_coef->getVectorCoefficient().get());
}
