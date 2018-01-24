/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "TestDiscontinuousValuePP.h"
#include "SolutionUserObject.h"

template <>
InputParameters
validParams<TestDiscontinuousValuePP>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  MooseEnum gradient_components("x=0 y=1 z=2", "x");
  params.addRequiredParam<VariableName>(
      "variable", "The name of the variable that this postprocessor operates on.");
  params.addRequiredParam<Point>("point",
                                 "The physical point where the solution will be evaluated.");
  params.addParam<bool>("evaluate_gradient",
                        false,
                        "Option to evaluate gradient instead of value. "
                        "If this is true, gradient_component must be "
                        "selected.");
  params.addParam<MooseEnum>(
      "gradient_component", gradient_components, "Component of gradient to be evaluated");
  params.addRequiredParam<UserObjectName>("solution",
                                          "The SolutionUserObject to extract data from.");
  return params;
}

TestDiscontinuousValuePP::TestDiscontinuousValuePP(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _variable_name(getParam<VariableName>("variable")),
    _point(getParam<Point>("point")),
    _evaluate_gradient(getParam<bool>("evaluate_gradient")),
    _gradient_component(getParam<MooseEnum>("gradient_component"))
{
}

void
TestDiscontinuousValuePP::initialSetup()
{
  _solution_object_ptr = &getUserObject<SolutionUserObject>("solution");
}

Real
TestDiscontinuousValuePP::getValue()
{
  if (_evaluate_gradient)
  {
    std::map<const Elem *, RealGradient> grad_map =
        _solution_object_ptr->discontinuousPointValueGradient(_t, _point, _variable_name);
    // If more than one then simply average
    Real grad = 0.0;
    for (auto & k : grad_map)
      grad += k.second(_gradient_component) / grad_map.size();
    return grad;
  }
  else
  {
    std::map<const Elem *, Real> value_map =
        _solution_object_ptr->discontinuousPointValue(_t, _point, _variable_name);
    // If more than one then simply average
    Real value = 0.0;
    for (auto & k : value_map)
      value += k.second / value_map.size();
    return value;
  }
}
