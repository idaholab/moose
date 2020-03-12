#include "SumPostprocessor.h"

registerMooseObject("THMApp", SumPostprocessor);

template <>
InputParameters
validParams<SumPostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<PostprocessorName>("a", "First postprocessor");
  params.addRequiredParam<PostprocessorName>("b", "Second postprocessor");

  return params;
}

SumPostprocessor::SumPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _sum(0.),
    _a(getPostprocessorValue("a")),
    _b(getPostprocessorValue("b"))
{
}

void
SumPostprocessor::initialize()
{
  _sum = 0;
}

void
SumPostprocessor::execute()
{
}

PostprocessorValue
SumPostprocessor::getValue()
{
  return _a + _b;
}
