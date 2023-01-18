//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeHomogenizedLagrangianStrainA.h"

registerMooseObject("TensorMechanicsTestApp", ComputeHomogenizedLagrangianStrainA);

InputParameters
ComputeHomogenizedLagrangianStrainA::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<MaterialPropertyName>("homogenization_gradient_name",
                                        "homogenization_gradient",
                                        "Name of the constant gradient field");
  params.addRequiredParam<MultiMooseEnum>(
      "constraint_types",
      HomogenizationB::constraintType,
      "Type of each constraint: strain, stress, or none. The types are specified in the "
      "column-major order, and there must be 9 entries in total.");
  params.addRequiredParam<std::vector<FunctionName>>(
      "targets", "Functions giving the targets to hit for constraint types that are not none.");
  params.addRequiredCoupledVar("macro_gradientA",
                               "Scalar field defining the 1st component of"
                               "macro gradient");
  params.addRequiredCoupledVar("macro_gradient",
                               "Scalar field defining the other components of"
                               "macro gradient");
  return params;
}

ComputeHomogenizedLagrangianStrainA::ComputeHomogenizedLagrangianStrainA(
    const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _macro_gradient(coupledScalarValue("macro_gradient")),
    _macro_gradientA(coupledScalarValue("macro_gradientA")),
    _homogenization_contribution(
        declareProperty<RankTwoTensor>(_base_name + "homogenization_gradient_name"))
{
  // Constraint types
  auto types = getParam<MultiMooseEnum>("constraint_types");
  if (types.size() != Moose::dim * Moose::dim)
    mooseError("Number of constraint types must equal dim * dim. ", types.size(), " are provided.");

  // Targets to hit
  const std::vector<FunctionName> & fnames = getParam<std::vector<FunctionName>>("targets");

  // Prepare the constraint map
  unsigned int fcount = 0;
  for (const auto j : make_range(Moose::dim))
    for (const auto i : make_range(Moose::dim))
    {
      const auto idx = i + Moose::dim * j;
      const auto ctype = static_cast<HomogenizationB::ConstraintType>(types.get(idx));
      if (ctype != HomogenizationB::ConstraintType::None)
      {
        const Function * const f = &getFunctionByName(fnames[fcount++]);
        _cmap[{i, j}] = {ctype, f};
      }
    }
}

void
ComputeHomogenizedLagrangianStrainA::computeQpProperties()
{
  _homogenization_contribution[_qp].zero();
  unsigned int count = 0;
  for (auto && indices : _cmap)
  {
    auto && [i, j] = indices.first;
    if (count == 0)
      _homogenization_contribution[_qp](i, j) = _macro_gradientA[count++];
    else
    {
      unsigned int r_ind = count - 1;
      count++;
      _homogenization_contribution[_qp](i, j) = _macro_gradient[r_ind];
    }
  }
}
