#include "VariableFunctionProductIC.h"
#include "Function.h"

registerMooseObject("THMApp", VariableFunctionProductIC);

InputParameters
VariableFunctionProductIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredCoupledVar("var", "Coupled variable");
  params.addRequiredParam<FunctionName>("fn", "User function");
  params.addClassDescription("Computes product of a variable and a function");
  return params;
}

VariableFunctionProductIC::VariableFunctionProductIC(const InputParameters & parameters)
  : InitialCondition(parameters), _var(coupledValue("var")), _fn(getFunction("fn"))
{
}

Real
VariableFunctionProductIC::value(const Point & p)
{
  return _var[_qp] * _fn.value(_t, p);
}
