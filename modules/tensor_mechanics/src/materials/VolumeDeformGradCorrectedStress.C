//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeDeformGradCorrectedStress.h"

template <>
InputParameters
validParams<VolumeDeformGradCorrectedStress>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription(
      "Transforms stress with volumetric term from previous configuration to this configuration");
  params.addRequiredParam<MaterialPropertyName>("pre_stress_name",
                                                "Name of stress variable from previous config.");
  params.addRequiredParam<MaterialPropertyName>("deform_grad_name",
                                                "Name of deformation gradient variable");
  params.addParam<MaterialPropertyName>("pre_jacobian_name",
                                        "Name of jacobian variable from previous config.");
  params.addRequiredParam<MaterialPropertyName>("stress_name", "Name of stress variable");
  params.addParam<MaterialPropertyName>("jacobian_name", "Name of jacobian variable");
  return params;
}

VolumeDeformGradCorrectedStress::VolumeDeformGradCorrectedStress(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _pre_stress(getMaterialProperty<RankTwoTensor>("pre_stress_name")),
    _deformation_gradient(getMaterialProperty<RankTwoTensor>("deform_grad_name")),
    _stress(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("stress_name")))
{
  if (isParamValid("pre_jacobian_name"))
    _pre_Jacobian_mult = &getMaterialProperty<RankFourTensor>("pre_jacobian_name");

  if (isParamValid("jacobian_name"))
    _Jacobian_mult =
        &declareProperty<RankFourTensor>(getParam<MaterialPropertyName>("jacobian_name"));
}

void
VolumeDeformGradCorrectedStress::initQpStatefulProperties()
{
  _stress[_qp] = _pre_stress[_qp];
}

void
VolumeDeformGradCorrectedStress::computeQpProperties()
{
  computeQpStress();
}

void
VolumeDeformGradCorrectedStress::computeQpStress()
{
  _stress[_qp] = _deformation_gradient[_qp] * _pre_stress[_qp] *
                 _deformation_gradient[_qp].transpose() / _deformation_gradient[_qp].det();

  if (isParamValid("pre_jacobian_name") && isParamValid("jacobian_name"))
    (*_Jacobian_mult)[_qp] = (*_pre_Jacobian_mult)[_qp];
}
