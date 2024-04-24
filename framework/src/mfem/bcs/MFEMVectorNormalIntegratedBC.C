#include "MFEMVectorNormalIntegratedBC.h"

registerMooseObject("PlatypusApp", MFEMVectorNormalIntegratedBC);

InputParameters
MFEMVectorNormalIntegratedBC::validParams()
{
  InputParameters params = MFEMBoundaryCondition::validParams();
  params.addRequiredParam<UserObjectName>(
      "vector_coefficient",
      "The vector MFEM coefficient whose normal component will be used in the integrated BC");
  return params;
}

// TODO: Currently assumes the vector function coefficient is 3D
MFEMVectorNormalIntegratedBC::MFEMVectorNormalIntegratedBC(const InputParameters & parameters)
  : MFEMBoundaryCondition(parameters),
    _vec_coef(const_cast<MFEMVectorCoefficient *>(
        &getUserObject<MFEMVectorCoefficient>("vector_coefficient")))
{

  _boundary_condition = std::make_shared<hephaestus::IntegratedBC>(
      getParam<std::string>("variable"),
      bdr_attr,
      std::make_unique<mfem::BoundaryNormalLFIntegrator>(*_vec_coef->getVectorCoefficient()));
}
