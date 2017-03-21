/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeDeformGradBasedStress.h"

template <>
InputParameters
validParams<ComputeDeformGradBasedStress>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Computes stress based on lagrangian strain");
  params.addRequiredParam<MaterialPropertyName>("deform_grad_name",
                                                "Name of deformation gradient variable");
  params.addRequiredParam<MaterialPropertyName>("elasticity_tensor_name",
                                                "Name of elasticity tensor variable");
  params.addRequiredParam<MaterialPropertyName>("stress_name", "Name of stress variable");
  params.addRequiredParam<MaterialPropertyName>("jacobian_name", "Name of jacobian variable");
  return params;
}

ComputeDeformGradBasedStress::ComputeDeformGradBasedStress(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _deformation_gradient(getMaterialProperty<RankTwoTensor>("deform_grad_name")),
    _elasticity_tensor(getMaterialProperty<RankFourTensor>("elasticity_tensor_name")),
    _stress(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("stress_name"))),
    _Jacobian_mult(declareProperty<RankFourTensor>(getParam<MaterialPropertyName>("jacobian_name")))
{
}

void
ComputeDeformGradBasedStress::initQpStatefulProperties()
{
  _stress[_qp].zero();
}

void
ComputeDeformGradBasedStress::computeQpProperties()
{
  computeQpStress();
}

void
ComputeDeformGradBasedStress::computeQpStress()
{
  const RankTwoTensor iden(RankTwoTensor::initIdentity);
  RankTwoTensor ee =
      0.5 * (_deformation_gradient[_qp].transpose() * _deformation_gradient[_qp] - iden);
  RankTwoTensor pk2 = _elasticity_tensor[_qp] * ee;

  _stress[_qp] = _deformation_gradient[_qp] * pk2 * _deformation_gradient[_qp].transpose() /
                 _deformation_gradient[_qp].det();
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}
