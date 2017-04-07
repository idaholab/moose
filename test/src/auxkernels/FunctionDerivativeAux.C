#include "FunctionDerivativeAux.h"
#include "Function.h"

template <>
InputParameters
validParams<FunctionDerivativeAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<FunctionName>("function", "The function to use as the value");
  params.addRequiredParam<unsigned int>("component", "What component to take the derivative with respect to. Should be either 1, 2, or 3.");
  return params;
}

FunctionDerivativeAux::FunctionDerivativeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _func(getFunction("function")),
    _component(getParam<unsigned int>("component"))
{
}

Real
FunctionDerivativeAux::computeValue()
{
  if (isNodal())
    return _func.gradient(_t, *_current_node)(_component - 1);
  else
    return _func.gradient(_t, _q_point[_qp])(_component - 1);
}
