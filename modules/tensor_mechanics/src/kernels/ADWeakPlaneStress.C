//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWeakPlaneStress.h"

registerMooseObject("TensorMechanicsApp", ADWeakPlaneStress);

InputParameters
ADWeakPlaneStress::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription("Plane stress kernel to provide out-of-plane strain contribution.");
  params.addParam<std::string>("base_name", "Material property base name");
  MooseEnum direction("x y z", "z");
  params.addParam<MooseEnum>("out_of_plane_strain_direction",
                             direction,
                             "The direction of the out-of-plane strain variable");
  params.set<bool>("use_displaced_mesh") = false;

  return params;
}

ADWeakPlaneStress::ADWeakPlaneStress(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(getADMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _direction(getParam<MooseEnum>("out_of_plane_strain_direction"))
{
}

ADReal
ADWeakPlaneStress::precomputeQpResidual()
{
  return _stress[_qp](_direction, _direction);
}
