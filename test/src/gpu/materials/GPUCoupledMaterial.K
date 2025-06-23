//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUCoupledMaterial.h"

registerMooseObject("MooseTestApp", GPUCoupledMaterial);

InputParameters
GPUCoupledMaterial::validParams()
{
  InputParameters params = GPUMaterial::validParams();
  params.addRequiredParam<MaterialPropertyName>("mat_prop",
                                                "Name of the property this material defines");
  params.addRequiredParam<MaterialPropertyName>(
      "coupled_mat_prop", "Name of the property to couple into this material");
  params.addParam<bool>(
      "use_old_prop",
      false,
      "Boolean indicating whether to use the old coupled property instead of the current property");
  return params;
}

GPUCoupledMaterial::GPUCoupledMaterial(const InputParameters & parameters)
  : GPUMaterial(parameters),
    _mat_prop_name(getParam<MaterialPropertyName>("mat_prop")),
    _mat_prop(declareGPUProperty<Real>(_mat_prop_name)),
    _coupled_mat_prop(getParam<bool>("use_old_prop")
                          ? getGPUMaterialPropertyOld<Real>("coupled_mat_prop")
                          : getGPUMaterialProperty<Real>("coupled_mat_prop"))
{
}
