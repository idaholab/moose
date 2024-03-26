//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BatchPropertyDerivativeTest.h"
#include "libmesh/int_range.h"

registerMooseObject("MooseTestApp", BatchPropertyDerivativeTest);

InputParameters
BatchPropertyDerivativeTest::validParams()
{
  auto params = BatchPropertyDerivativeTestParent::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop", "A RankTwoTensor property");
  params.addRequiredParam<UserObjectName>("batch_deriv_uo",
                                          "A batch property derivative userObject");
  return params;
}

BatchPropertyDerivativeTest::BatchPropertyDerivativeTest(const InputParameters & params)
  : BatchPropertyDerivativeTestParent(
        params,
        // here we pass in the parameter names of the material property
        "prop")
{
  const auto prop_value = getParam<UserObjectName>("batch_deriv_uo");
  _batch_prop_deriv_uo = const_cast<BatchPropertyDerivativeRankTwoTensorReal *>(
      &getUserObjectByName<BatchPropertyDerivativeRankTwoTensorReal>(prop_value));
}

void
BatchPropertyDerivativeTest::batchCompute()
{
  auto & _deriv_uo_output = _batch_prop_deriv_uo->setOutputData();
  for (const auto i : index_range(_deriv_uo_output))
  {
    const auto & input = _input_data[i];
    auto & output = _output_data[i];

    const auto & [prop] = input;

    // Fill the UO with customized value
    _deriv_uo_output[i] = prop;

    // Set the batch UO output value
    output = _deriv_uo_output[i].L2norm();
  }
}
