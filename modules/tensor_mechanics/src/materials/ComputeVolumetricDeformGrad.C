/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeVolumetricDeformGrad.h"

template <>
InputParameters
validParams<ComputeVolumetricDeformGrad>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription(
      "Computes volumetric deformation gradient and adjusts the total deformation gradient");
  params.addRequiredParam<MaterialPropertyName>(
      "pre_deform_grad_name", "Name of pre-adjusted deformation gradient variable");
  params.addRequiredParam<MaterialPropertyName>("volumetric_deform_grad_name",
                                                "Name of volumetric deformation gradient variable");
  params.addRequiredParam<MaterialPropertyName>("post_deform_grad_name",
                                                "Name of adjusted deformation gradient variable");
  return params;
}

ComputeVolumetricDeformGrad::ComputeVolumetricDeformGrad(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _pre_deform_grad(getMaterialProperty<RankTwoTensor>("pre_deform_grad_name")),
    _volumetric_deform_grad(declareProperty<RankTwoTensor>(
        getParam<MaterialPropertyName>("volumetric_deform_grad_name"))),
    _post_deform_grad(
        declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("post_deform_grad_name")))
{
}

void
ComputeVolumetricDeformGrad::initQpStatefulProperties()
{
  _volumetric_deform_grad[_qp].zero();
  _volumetric_deform_grad[_qp].addIa(1.0);

  _post_deform_grad[_qp].zero();
  _post_deform_grad[_qp].addIa(1.0);
}

void
ComputeVolumetricDeformGrad::computeQpProperties()
{
  createVolumetricDeformGrad();
  _post_deform_grad[_qp] = _volumetric_deform_grad[_qp].inverse() * _pre_deform_grad[_qp];
}

void
ComputeVolumetricDeformGrad::createVolumetricDeformGrad()
{
  _volumetric_deform_grad[_qp].zero();
  _volumetric_deform_grad[_qp].addIa(1.0);
}
