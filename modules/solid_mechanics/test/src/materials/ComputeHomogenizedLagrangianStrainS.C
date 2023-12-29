//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeHomogenizedLagrangianStrainS.h"

registerMooseObject("TensorMechanicsTestApp", ComputeHomogenizedLagrangianStrainS);

InputParameters
ComputeHomogenizedLagrangianStrainS::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<MaterialPropertyName>("homogenization_gradient_name",
                                        "homogenization_gradient",
                                        "Name of the constant gradient field");
  params.addRequiredParam<MultiMooseEnum>(
      "constraint_types",
      HomogenizationM::constraintType,
      "Type of each constraint: strain, stress, or none. The types are specified in the "
      "column-major order, and there must be 9 entries in total.");
  params.addRequiredParam<std::vector<FunctionName>>(
      "targets", "Functions giving the targets to hit for constraint types that are not none.");
  params.addRequiredCoupledVar("macro_gradient",
                               "Scalar field defining the "
                               "macro gradient");
  return params;
}

ComputeHomogenizedLagrangianStrainS::ComputeHomogenizedLagrangianStrainS(
    const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _macro_gradient(coupledScalarValue("macro_gradient")),
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
      const auto ctype = static_cast<HomogenizationM::ConstraintType>(types.get(idx));
      if (ctype != HomogenizationM::ConstraintType::None)
      {
        const Function * const f = &getFunctionByName(fnames[fcount++]);
        _cmap[{i, j}] = {ctype, f};
      }
    }
}

void
ComputeHomogenizedLagrangianStrainS::computeQpProperties()
{
  _homogenization_contribution[_qp].zero();
  unsigned int count = 0;
  for (auto && indices : _cmap)
  {
    auto && [i, j] = indices.first;
    _homogenization_contribution[_qp](i, j) = _macro_gradient[count++];
  }
}
