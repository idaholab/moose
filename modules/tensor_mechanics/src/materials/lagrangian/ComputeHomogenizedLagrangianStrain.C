//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeHomogenizedLagrangianStrain.h"

registerMooseObject("TensorMechanicsApp", ComputeHomogenizedLagrangianStrain);

InputParameters
ComputeHomogenizedLagrangianStrain::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("base_name", "Material property base name");
  params.addRequiredParam<UserObjectName>(
      "homogenization_constraint", "The UserObject for defining the homogenization constraint");
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
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _constraint(getUserObject<HomogenizationConstraint>("homogenization_constraint")),
    _cmap(_constraint.getConstraintMap()),
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
  for (auto && indices : _cmap)
  {
    auto && [i, j] = indices.first;
    _homogenization_contribution[_qp](i, j) = _macro_gradient[count++];
  }
}
