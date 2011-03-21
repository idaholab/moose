#include "SideIntegral.h"

template<>
InputParameters validParams<SideIntegral>()
{
  InputParameters params = validParams<SidePostprocessor>();
  return params;
}

SideIntegral::SideIntegral(const std::string & name, InputParameters parameters) :
    SidePostprocessor(name, parameters),
    _integral_value(0)
{}

void
SideIntegral::initialize()
{
  _integral_value = 0;
}

void
SideIntegral::execute()
{
  _integral_value += computeIntegral();
}

Real
SideIntegral::getValue()
{
  gatherSum(_integral_value);
  return _integral_value;
}

Real
SideIntegral::computeQpIntegral()
{
  return _u[_qp];
}
