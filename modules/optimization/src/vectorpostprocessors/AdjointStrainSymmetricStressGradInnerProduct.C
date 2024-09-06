//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdjointStrainSymmetricStressGradInnerProduct.h"

registerMooseObject("OptimizationApp", AdjointStrainSymmetricStressGradInnerProduct);

InputParameters
AdjointStrainSymmetricStressGradInnerProduct::validParams()
{
  InputParameters params = ElementOptimizationFunctionInnerProduct::validParams();
  params.addClassDescription(
      "This component is designed to compute the gradient of the objective function concerning "
      "specific properties. It achieves this by computing the inner product of the property "
      "derivative obtained a material property and the strain resulting from the forward "
      "simulation.");
  params.addRequiredParam<MaterialPropertyName>(
      "stress_derivative_name", "The name of the stress derivative material property");
  params.addRequiredParam<MaterialPropertyName>(
      "adjoint_strain_name", "Name of the strain property in the adjoint problem");
  return params;
}

AdjointStrainSymmetricStressGradInnerProduct::AdjointStrainSymmetricStressGradInnerProduct(
    const InputParameters & parameters)
  : ElementOptimizationFunctionInnerProduct(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress_derivative(getMaterialPropertyByName<SymmetricRankTwoTensor>(
        getParam<MaterialPropertyName>("stress_derivative_name"))),
    _adjoint_strain(getMaterialPropertyByName<RankTwoTensor>(
        getParam<MaterialPropertyName>("adjoint_strain_name")))
{
}

Real
AdjointStrainSymmetricStressGradInnerProduct::computeQpInnerProduct()
{
  auto derivative = RankTwoTensor(_stress_derivative[_qp]);
  return -_adjoint_strain[_qp].doubleContraction(derivative);
}
