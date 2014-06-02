#include "DifferencePostprocessor.h"

template<>
InputParameters validParams<DifferencePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<PostprocessorName>("value1", "First value");
  params.addRequiredParam<PostprocessorName>("value2", "Second value");

  return params;
}

DifferencePostprocessor::DifferencePostprocessor(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _value1(getPostprocessorValue("value1")),
    _value2(getPostprocessorValue("value2"))
{
}

DifferencePostprocessor::~DifferencePostprocessor()
{
}

void
DifferencePostprocessor::initialize()
{
}

void
DifferencePostprocessor::execute()
{
}

PostprocessorValue
DifferencePostprocessor::getValue()
{
  return _value1 - _value2;
}

void
DifferencePostprocessor::threadJoin(const UserObject & /*uo*/)
{
  // nothing to do here, general PPS do not run threaded
}
