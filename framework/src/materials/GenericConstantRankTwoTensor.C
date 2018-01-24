/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
