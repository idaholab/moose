#include "ElementIntegral.h"

template<>
InputParameters validParams<ElementIntegral>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  return params;
}

ElementIntegral::ElementIntegral(const std::string & name, InputParameters parameters) :
    ElementPostprocessor(name, parameters),
    _integral_value(0)
{}

void
ElementIntegral::initialize()
{
  _integral_value = 0;
}

void
ElementIntegral::execute()
{
  // FIXME: computeItegral
//  _integral_value += computeIntegral();
}

Real
ElementIntegral::getValue()
{
  gatherSum(_integral_value);
  return _integral_value;
}

Real
ElementIntegral::computeQpIntegral()
{
  return _u[_qp];
}
