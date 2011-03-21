#include "AverageElementSize.h"

template<>
InputParameters validParams<AverageElementSize>()
{
  InputParameters params = validParams<ElementAverageValue>();
  return params;
}

AverageElementSize::AverageElementSize(const std::string & name, InputParameters parameters) :
    ElementAverageValue(name, parameters)
{}

void
AverageElementSize::initialize()
{
  ElementAverageValue::initialize();
  _elems = 0;
}

void
AverageElementSize::execute()
{
  ElementIntegral::execute();
  _elems ++;
}

Real
AverageElementSize::computeIntegral()
{
  return _current_elem->hmax();
}

Real
AverageElementSize::getValue()
{
  Real integral = ElementIntegral::getValue();

  gatherSum(_elems);

  return integral / _elems;
}
