#include "StVenantKirchhoffStressUpdate.h"

registerMooseObject("TensorMechanicsApp", StVenantKirchhoffStressUpdate);

InputParameters
StVenantKirchhoffStressUpdate::validParams()
{
  InputParameters params = ComputeLagrangianStressPK2::validParams();

  params.addParam<MaterialPropertyName>(
      "elasticity_tensor", "elasticity_tensor", "The name of the elasticity tensor.");

  return params;
}

StVenantKirchhoffStressUpdate::StVenantKirchhoffStressUpdate(const InputParameters & parameters)
  : ComputeLagrangianStressPK2(parameters),
    GuaranteeConsumer(this),
    _elasticity_tensor_name(getParam<MaterialPropertyName>("elasticity_tensor")),
    _elasticity_tensor(getMaterialProperty<RankFourTensor>(_elasticity_tensor_name))
{
}

void
StVenantKirchhoffStressUpdate::initialSetup()
{
  // Enforce isotropic elastic tensor
  if (!hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC))
    mooseError("StVenantKirchhoffStressUpdate requires an isotropic elasticity "
               "tensor");
}

void
StVenantKirchhoffStressUpdate::computeQpPK2Stress()
{
  // Hyperelasticity is weird, we need to branch on the type of update if we
  // want a truly linear model
  //
  // This is because we need to drop quadratic terms for the linear update to
  // use a linear strain measure

  // Jacobian is the same for both
  _C[_qp] = _elasticity_tensor[_qp];

  // Get the right strain
  RankTwoTensor strain;
  if (_large_kinematics) // Large deformations = nonlinear strain
    strain = _E[_qp];
  else // Small deformations = linear strain
    strain = 0.5 * (_F[_qp] + _F[_qp].transpose());

  // The stress update is linear with the correct strains/frame
  _S[_qp] = _C[_qp] * strain;
}
