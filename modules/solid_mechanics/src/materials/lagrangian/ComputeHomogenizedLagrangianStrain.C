//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeHomogenizedLagrangianStrain.h"

registerMooseObject("SolidMechanicsApp", ComputeHomogenizedLagrangianStrain);

InputParameters
ComputeHomogenizedLagrangianStrain::validParams()
{
  InputParameters params = HomogenizationInterface<Material>::validParams();
  params.addClassDescription("Calculate eigenstrain-like contribution from the homogenization "
                             "strain used to satisfy the homogenization constraints.");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<MaterialPropertyName>("homogenization_gradient_name",
                                        "homogenization_gradient",
                                        "Name of the constant gradient field");
  params.addRequiredCoupledVar("macro_gradient",
                               "Scalar field defining the "
                               "macro gradient");
  return params;
}

ComputeHomogenizedLagrangianStrain::ComputeHomogenizedLagrangianStrain(
    const InputParameters & parameters)
  : HomogenizationInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _macro_gradient(coupledScalarValue("macro_gradient")),
    _homogenization_contribution(
        declareProperty<RankTwoTensor>(_base_name + "homogenization_gradient_name"))
{
}

void
ComputeHomogenizedLagrangianStrain::computeQpProperties()
{
  _homogenization_contribution[_qp].zero();
  unsigned int count = 0;
  for (const auto & [indices, constraint] : cmap())
  {
    const auto [i, j] = indices;
    _homogenization_contribution[_qp](i, j) = _macro_gradient[count++];
  }
}
