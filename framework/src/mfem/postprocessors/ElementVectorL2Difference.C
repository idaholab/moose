#include "ElementVectorL2Difference.h"

registerMooseObject("MooseApp", ElementVectorL2Difference);

InputParameters
ElementVectorL2Difference::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();

  params.addRequiredCoupledVar("variable", "The name of the vector variable.");
  params.addRequiredCoupledVar("other_variable",
                               "The name of the other vector variable to compare against.");

  params.addClassDescription(
      "Computes the element-wise L2 difference between two coupled vector fields.");
  return params;
}

ElementVectorL2Difference::ElementVectorL2Difference(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _vector_variable(coupledVectorValue("variable")),
    _other_vector_variable(coupledVectorValue("other_variable"))
{
  checkVectorVariables();
}

Real
ElementVectorL2Difference::getValue() const
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
ElementVectorL2Difference::computeQpIntegral()
{
  RealVectorValue solution_value(0.0, 0.0, 0.0);
  RealVectorValue other_value(0.0, 0.0, 0.0);

  for (int icomponent = 0; icomponent < 3; icomponent++)
  {
    solution_value(icomponent) = _vector_variable[_qp](icomponent);
    other_value(icomponent) = _other_vector_variable[_qp](icomponent);
  }

  RealVectorValue difference_vector = (solution_value - other_value);

  return difference_vector.norm_sq(); // dot product of difference vector.
}

void
ElementVectorL2Difference::checkVectorVariables() const
{
  auto & coupled_vector_variables = getCoupledVectorMooseVars();

  if (coupled_vector_variables.size() != 2)
  {
    mooseError(
        "There are ", coupled_vector_variables.size(), " coupled vector variables. Expected 2.");
  }

  auto first = coupled_vector_variables[0];
  auto second = coupled_vector_variables[1];

  if (!first || !second)
  {
    mooseError("Coupled vector variable is nullptr.");
  }

  if (first->feType().family != second->feType().family)
  {
    mooseError("The families of the vector variables must match.");
  }

  if (first->order() != second->order())
  {
    mooseError("The order of the vector variables must match.");
  }
}
