//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericFunctionRankTwoTensor.h"

#include "Function.h"

registerMooseObject("MooseApp", GenericFunctionRankTwoTensor);
registerMooseObject("MooseApp", ADGenericFunctionRankTwoTensor);

template <bool is_ad>
InputParameters
GenericFunctionRankTwoTensorTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Material object for defining rank two tensor properties using functions.");
  params.addRequiredParam<std::vector<FunctionName>>(
      "tensor_functions", "Vector of Function names defining the rank two tensor");
  params.addRequiredParam<MaterialPropertyName>(
      "tensor_name", "Name of the tensor material property to be created");
  return params;
}

template <bool is_ad>
GenericFunctionRankTwoTensorTempl<is_ad>::GenericFunctionRankTwoTensorTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _prop(declareGenericProperty<RankTwoTensor, is_ad>(
        getParam<MaterialPropertyName>("tensor_name"))),
    _function_names(getParam<std::vector<FunctionName>>("tensor_functions")),
    _num_functions(_function_names.size()),
    _functions(_num_functions)
{
  for (unsigned int i = 0; i < _num_functions; i++)
    _functions[i] = &getFunctionByName(_function_names[i]);
}

template <bool is_ad>
void
GenericFunctionRankTwoTensorTempl<is_ad>::initQpStatefulProperties()
{
  GenericFunctionRankTwoTensorTempl<is_ad>::computeQpProperties();
}

template <bool is_ad>
void
GenericFunctionRankTwoTensorTempl<is_ad>::computeQpProperties()
{
  std::vector<GenericReal<is_ad>> values(_num_functions);
  for (unsigned int i = 0; i < _num_functions; i++)
    values[i] = (*_functions[i]).value(_t, _q_point[_qp]);

  _prop[_qp].fillFromInputVector(values);
}
