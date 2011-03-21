#include "Reporter.h"

template<>
InputParameters validParams<Reporter>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

Reporter::Reporter(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _my_value(getPostprocessorValue(name))
{}

Real
Reporter::getValue()
{
  return _my_value;
}
