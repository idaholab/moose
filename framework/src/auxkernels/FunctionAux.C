#include "FunctionAux.h"
#include "Function.h"

template<>
InputParameters validParams<FunctionAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("function", "The function to use as the value");
  return params;
}

FunctionAux::FunctionAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _func(getFunction("function"))
{
}

Real
FunctionAux::computeValue()
{
  return _func.value(_t, *_current_node);
}
