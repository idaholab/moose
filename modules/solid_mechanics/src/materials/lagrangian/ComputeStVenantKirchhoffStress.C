//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeStVenantKirchhoffStress.h"

registerMooseObject("TensorMechanicsApp", ComputeStVenantKirchhoffStress);

InputParameters
ComputeStVenantKirchhoffStress::validParams()
{
  InputParameters params = ComputeLagrangianStressPK2::validParams();

  params.addParam<MaterialPropertyName>(
      "elasticity_tensor", "elasticity_tensor", "The name of the elasticity tensor.");

  return params;
}

ComputeStVenantKirchhoffStress::ComputeStVenantKirchhoffStress(const InputParameters & parameters)
  : ComputeLagrangianStressPK2(parameters),
    GuaranteeConsumer(this),
    _elasticity_tensor_name(getParam<MaterialPropertyName>("elasticity_tensor")),
    _elasticity_tensor(getMaterialProperty<RankFourTensor>(_elasticity_tensor_name))
{
}

void
ComputeStVenantKirchhoffStress::initialSetup()
{
  // Enforce isotropic elastic tensor
  if (!hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC))
    mooseError("ComputeStVenantKirchhoffStress requires an isotropic elasticity "
               "tensor");
}

void
ComputeStVenantKirchhoffStress::computeQpPK2Stress()
{
  // Hyperelasticity is weird, we need to branch on the type of update if we
  // want a truly linear model
  //
  // This is because we need to drop quadratic terms for the linear update to
  // use a linear strain measure

  // Jacobian is the same for both the small and Green-Lagrange strains
  _C[_qp] = _elasticity_tensor[_qp];

  // Get the right strain
  RankTwoTensor strain;
  if (_large_kinematics) // Large deformations = Green-Lagrange strain
    strain = _E[_qp];
  else // Small deformations = linear strain
    strain = 0.5 * (_F[_qp] + _F[_qp].transpose()) - RankTwoTensor::Identity();

  // The stress update is linear with the correct strains/frame
  _S[_qp] = _C[_qp] * strain;
}
