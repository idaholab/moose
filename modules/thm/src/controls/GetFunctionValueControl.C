#include "GetFunctionValueControl.h"
#include "Function.h"

registerMooseObject("RELAP7App", GetFunctionValueControl);

template <>
InputParameters
validParams<GetFunctionValueControl>()
{
  InputParameters params = validParams<RELAP7Control>();
  params.addRequiredParam<FunctionName>("function",
                                        "The name of the function prescribing a value.");
  return params;
}

GetFunctionValueControl::GetFunctionValueControl(const InputParameters & parameters)
  : RELAP7Control(parameters),
    _value(declareComponentControlData<Real>("value")),
    _function(getFunction("function"))
{
}

void
GetFunctionValueControl::execute()
{
  _value = _function.value(_t, Point());
}
