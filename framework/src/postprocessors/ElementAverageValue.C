#include "ElementAverageValue.h"

template<>
InputParameters validParams<ElementAverageValue>()
{
  InputParameters params = validParams<ElementIntegral>();
  return params;
}

ElementAverageValue::ElementAverageValue(const std::string & name, InputParameters parameters) :
    ElementIntegral(name, parameters),
    _volume(0)
{}

void
ElementAverageValue::initialize()
{
  ElementIntegral::initialize();
  
  _volume = 0;
}

void
ElementAverageValue::execute()
{
  ElementIntegral::execute();

  _volume += _current_elem->volume();
}

Real
ElementAverageValue::getValue()
{
  Real integral = ElementIntegral::getValue();
  
  gatherSum(_volume);
  
  return integral / _volume;
}
