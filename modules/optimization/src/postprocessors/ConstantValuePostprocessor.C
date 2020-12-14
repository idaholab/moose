#include "ConstantValuePostprocessor.h"
#include "Function.h"

registerMooseObject("isopodApp", ConstantValuePostprocessor);

defineLegacyParams(ConstantValuePostprocessor);

InputParameters
ConstantValuePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addParam<Real>("value", 1, "Value of the parameter to be applied to the function.");
  params.declareControllable("value");
  params.addClassDescription("Displays a constant value.");
  return params;
}

ConstantValuePostprocessor::ConstantValuePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _value(getParam<Real>("value"))
{
}

void
ConstantValuePostprocessor::initialize()
{
}

void
ConstantValuePostprocessor::execute()
{
}

PostprocessorValue
ConstantValuePostprocessor::getValue()
{
  return _value;
}
