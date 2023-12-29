//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SumTensorIncrements.h"
#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", SumTensorIncrements);

InputParameters
SumTensorIncrements::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute tensor property by summing tensor increments");
  params.addRequiredParam<MaterialPropertyName>("tensor_name", "Name of strain property");
  params.addParam<std::vector<MaterialPropertyName>>("coupled_tensor_increment_names",
                                                     "Name of strain increment properties");
  return params;
}

SumTensorIncrements::SumTensorIncrements(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _property_names(getParam<std::vector<MaterialPropertyName>>("coupled_tensor_increment_names")),
    _tensor(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("tensor_name"))),
    _tensor_old(
        getMaterialPropertyOld<RankTwoTensor>(getParam<MaterialPropertyName>("tensor_name"))),
    _tensor_increment(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("tensor_name") +
                                                     "_increment"))
{
  _num_property = _property_names.size();

  if (_num_property > 0)
  {
    _coupled_tensor_increments.resize(_num_property);

    for (unsigned int i = 0; i < _num_property; ++i)
      _coupled_tensor_increments[i] = &getMaterialProperty<RankTwoTensor>(_property_names[i]);
  }
}

void
SumTensorIncrements::initQpStatefulProperties()
{
  _tensor[_qp].zero();
  _tensor_increment[_qp].zero();
}

void
SumTensorIncrements::computeQpProperties()
{
  _tensor_increment[_qp].zero();

  for (unsigned int i = 0; i < _num_property; ++i)
    _tensor_increment[_qp] += (*_coupled_tensor_increments[i])[_qp];

  _tensor[_qp] = _tensor_old[_qp] + _tensor_increment[_qp];
}
