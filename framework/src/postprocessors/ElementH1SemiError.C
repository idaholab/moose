#include "ElementH1SemiError.h"
#include "Function.h"

template<>
InputParameters validParams<ElementH1SemiError>()
{
  InputParameters params = validParams<ElementIntegral>();
  params.addRequiredParam<std::string>("function", "The analytic solution to compare against");
  return params;
}

ElementH1SemiError::ElementH1SemiError(const std::string & name, InputParameters parameters) :
    ElementIntegral(name, parameters),
    FunctionInterface(parameters),
    _func(getFunction("function"))
{
}

Real
ElementH1SemiError::getValue()
{
  return std::sqrt(ElementIntegral::getValue());
}

Real
ElementH1SemiError::computeQpIntegral()
{
  RealGradient diff = _grad_u[_qp]-_func.gradient(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
  return diff*diff;
}
