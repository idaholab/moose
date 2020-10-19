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

defineLegacyParams(GenericFunctionRankTwoTensor);

InputParameters
GenericFunctionRankTwoTensor::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<std::vector<FunctionName>>(
      "tensor_functions", "Vector of Function names defining the rank two tensor");
  params.addRequiredParam<MaterialPropertyName>(
      "tensor_name", "Name of the tensor material property to be created");
  return params;
}

GenericFunctionRankTwoTensor::GenericFunctionRankTwoTensor(const InputParameters & parameters)
  : Material(parameters),
    _prop(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("tensor_name"))),
    _function_names(getParam<std::vector<FunctionName>>("tensor_functions")),
    _num_functions(_function_names.size()),
    _functions(_num_functions)
{
  for (unsigned int i = 0; i < _num_functions; i++)
    _functions[i] = &getFunctionByName(_function_names[i]);
}

void
GenericFunctionRankTwoTensor::computeQpProperties()
{
  std::vector<Real> values(_num_functions);
  for (unsigned int i = 0; i < _num_functions; i++)
    values[i] = (*_functions[i]).value(_t, _q_point[_qp]);

  _prop[_qp].fillFromInputVector(values);
}
