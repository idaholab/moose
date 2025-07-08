#include "TwoValuesL2Norm.h"

registerMooseObject("MooseApp", TwoValuesL2Norm);

InputParameters
TwoValuesL2Norm::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addRequiredCoupledVar("other_variable", "The second variable");
  return params;
}

TwoValuesL2Norm::TwoValuesL2Norm(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
	_other_var(coupledValue("other_variable"))
{
}

Real
TwoValuesL2Norm::getValue() const
{
  return std::sqrt(ElementIntegralVariablePostprocessor::getValue());
}

Real
TwoValuesL2Norm::computeQpIntegral()
{
  Real value = _u[_qp]*_other_var[_qp];
  return value;
}
