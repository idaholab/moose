//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericConstantRankTwoTensor.h"

template <>
InputParameters
validParams<GenericConstantRankTwoTensor>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::vector<Real>>(
      "tensor_values", "Vector of values defining the constant rank two tensor");
  params.addRequiredParam<MaterialPropertyName>(
      "tensor_name", "Name of the tensor material property to be created");
  return params;
}

GenericConstantRankTwoTensor::GenericConstantRankTwoTensor(const InputParameters & parameters)
  : Material(parameters),
    _prop(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("tensor_name")))
{
  _tensor.fillFromInputVector(getParam<std::vector<Real>>("tensor_values"));
}

void
GenericConstantRankTwoTensor::computeQpProperties()
{
  _prop[_qp] = _tensor;
}
