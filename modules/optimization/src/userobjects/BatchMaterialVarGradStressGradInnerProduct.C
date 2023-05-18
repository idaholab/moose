//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BatchMaterialVarGradStressGradInnerProduct.h"
#include "libmesh/int_range.h"

registerMooseObject("OptimizationApp", BatchMaterialVarGradStressGradInnerProduct);

InputParameters
BatchMaterialVarGradStressGradInnerProduct::validParams()
{
  auto params = BatchMaterialVarGradStressGradInnerProductParent::validParams();
  params.addRequiredCoupledVar("adjoint_var", "The adjoint variable");
  params.addRequiredParam<MaterialPropertyName>("material_grad", "Gradient (of diffusion/Young's modulus) with respect to one parameter");
  return params;
}

BatchMaterialVarGradStressGradInnerProduct::BatchMaterialVarGradStressGradInnerProduct(const InputParameters & params)
  : BatchMaterialVarGradStressGradInnerProductParent(
        params,
        // here we pass in the parameter names of the variable and the two material properties
        // in the same order as in the template parameter pack
        "adjoint_var",
        "material_grad")
{
}

void
BatchMaterialVarGradStressGradInnerProduct::batchCompute()
{
  for (const auto i : index_range(_input_data))
  {
    const auto & input = _input_data[i];
    auto & output = _output_data[i];

    const auto & adjoint_grad = std::get<0>(input);
    const auto & material_grad = std::get<1>(input);

    output = -adjoint_grad * material_grad;
  }
}
