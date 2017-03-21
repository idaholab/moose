/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SumTensorIncrements.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<SumTensorIncrements>()
{
  InputParameters params = validParams<Material>();
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
    _tensor_old(declarePropertyOld<RankTwoTensor>(getParam<MaterialPropertyName>("tensor_name"))),
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
