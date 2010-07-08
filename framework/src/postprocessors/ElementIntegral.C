#include "ElementIntegral.h"

template<>
InputParameters validParams<ElementIntegral>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  return params;
}

ElementIntegral::ElementIntegral(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :ElementPostprocessor(name, moose_system, parameters),
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
  _integral_value += computeIntegral();
}

Real
ElementIntegral::getValue()
{
  return _integral_value;
}

Real
ElementIntegral::computeQpIntegral()
{
  return _u[_qp];
}
