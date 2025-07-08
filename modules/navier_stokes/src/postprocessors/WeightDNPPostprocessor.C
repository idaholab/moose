#include "WeightDNPPostprocessor.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", WeightDNPPostprocessor);

InputParameters
WeightDNPPostprocessor::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addClassDescription("Calculates the weight of the DNPs");
  params.addRequiredCoupledVar("other_variable", "The variable to compare to");
  params.addParam<PostprocessorName>("Norm", 1.0, "The squareroot of the product of the weight functions");
  params.addParam<Real>("lambda", 1.0, "lambda");
  return params;
}

WeightDNPPostprocessor::WeightDNPPostprocessor(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
	_other_var(coupledValue("other_variable")),
	_norm(getPostprocessorValue("Norm")),
    _lambda(getParam<Real>("lambda"))
{
}

Real
WeightDNPPostprocessor::getValue() const
{
  //return ElementIntegralVariablePostprocessor::getValue()*_lambda/(_norm);
 return ElementIntegralVariablePostprocessor::getValue();
}

Real
WeightDNPPostprocessor::computeQpIntegral()
{
  Real prod = _u[_qp] * _other_var[_qp];
  return prod*_lambda/(_norm*_norm);
}

