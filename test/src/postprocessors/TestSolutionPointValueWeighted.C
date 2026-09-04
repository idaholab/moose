//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestSolutionPointValueWeighted.h"

#include "SolutionUserObjectBase.h"

registerMooseObject("MooseTestApp", TestSolutionPointValueWeighted);

InputParameters
TestSolutionPointValueWeighted::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  const MooseEnum gradient_components("x=0 y=1 z=2", "x");

  params.addRequiredParam<VariableName>(
      "variable", "The variable in the imported source solution to evaluate.");
  params.addRequiredParam<Point>("point",
                                 "The physical point at which to evaluate the imported solution.");
  params.addParam<bool>(
      "evaluate_gradient", false, "Whether to evaluate a gradient instead of a value.");
  params.addParam<MooseEnum>("gradient_component",
                             gradient_components,
                             "The gradient component returned when evaluating a gradient.");
  params.addParam<MooseEnum>(
      "weighting_type",
      SolutionUserObjectBase::weightingType(),
      "The policy used to select a unique result when the source field is multivalued.");
  params.addParam<std::vector<subdomain_id_type>>(
      "source_subdomain_ids",
      {},
      "Optional numeric subdomain IDs in the imported source mesh on which to search.");
  params.addRequiredParam<UserObjectName>(
      "solution", "The SolutionUserObject containing the imported source solution.");

  return params;
}

TestSolutionPointValueWeighted::TestSolutionPointValueWeighted(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _variable_name(getParam<VariableName>("variable")),
    _point(getParam<Point>("point")),
    _evaluate_gradient(getParam<bool>("evaluate_gradient")),
    _gradient_component(getParam<MooseEnum>("gradient_component")),
    _weighting_type(
        getParam<MooseEnum>("weighting_type").getEnum<SolutionUserObjectBase::WeightingType>()),
    _source_subdomain_ids(getParam<std::vector<subdomain_id_type>>("source_subdomain_ids"))
{
}

void
TestSolutionPointValueWeighted::initialSetup()
{
  _solution_object_ptr = &getUserObject<SolutionUserObjectBase>("solution");
}

Real
TestSolutionPointValueWeighted::getValue() const
{
  const std::set<subdomain_id_type> source_subdomain_ids(_source_subdomain_ids.begin(),
                                                         _source_subdomain_ids.end());

  const auto * const source_subdomain_ids_ptr =
      source_subdomain_ids.empty() ? nullptr : &source_subdomain_ids;

  if (_evaluate_gradient)
    return _solution_object_ptr->pointValueGradient(
        _t, _point, _variable_name, _weighting_type, source_subdomain_ids_ptr)(_gradient_component);

  return _solution_object_ptr->pointValue(
      _t, _point, _variable_name, _weighting_type, source_subdomain_ids_ptr);
}
