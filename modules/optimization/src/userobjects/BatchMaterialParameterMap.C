//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BatchMaterialParameterMap.h"
#include "libmesh/int_range.h"

registerMooseObject("OptimizationApp", BatchMaterialParameterMap);

InputParameters
BatchMaterialParameterMap::validParams()
{
  auto params = BatchMaterialParameterMapParent::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop1", "A real property");
  params.addParam<std::vector<FunctionName>>("property_values",
                                             "The corresponding names of the "
                                             "functions that are going to provide "
                                             "the values for the variables");
  params.addParam<std::vector<std::string>>(
      "property_names", "The names of the properties the batch material will have");

  return params;
}

BatchMaterialParameterMap::BatchMaterialParameterMap(const InputParameters & params)
  : BatchMaterialParameterMapParent(
        params,
        // here we pass in the parameter names of the variable and the two material properties
        // in the same order as in the template parameter pack
        "prop1")
{
}

void
BatchMaterialParameterMap::batchCompute()
{
  // simple test computation
  for (const auto i : index_range(_input_data))
  {
    const auto & input = _input_data[i];
    auto & output = _output_data[i];

    const auto & prop1 = std::get<0>(input);

    output = prop1;
  }
}
