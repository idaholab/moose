//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BatchMaterialTest.h"
#include "libmesh/int_range.h"

registerMooseObject("MooseTestApp", BatchMaterialTest);

InputParameters
BatchMaterialTest::validParams()
{
  auto params = BatchMaterialTestParent::validParams();
  params.addCoupledVar("var1", "A coupled variable");
  params.addRequiredParam<MaterialPropertyName>("prop1", "A RankTwoTensor property");
  params.addRequiredParam<MaterialPropertyName>("prop2", "A Real property");
  return params;
}

BatchMaterialTest::BatchMaterialTest(const InputParameters & params)
  : BatchMaterialTestParent(
        params,
        // here we pass in the parameter names of the variable and the two material properties
        // in the same order as in the template parameter pack
        "var1",
        "prop1",
        "prop2")
{
}

void
BatchMaterialTest::batchCompute()
{
  // simple test computation
  for (const auto i : index_range(_input_data))
  {
    const auto & input = _input_data[i];
    auto & output = _output_data[i];

    const auto & var1 = std::get<0>(input);
    const auto & prop1 = std::get<1>(input);
    const auto & prop2 = std::get<2>(input);

    output = var1 * prop1.L2norm() + prop2;
  }
}
