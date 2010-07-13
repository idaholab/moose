#include "SideIntegral.h"

template<>
InputParameters validParams<SideIntegral>()
{
  InputParameters params = validParams<SidePostprocessor>();
  return params;
}

SideIntegral::SideIntegral(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :SidePostprocessor(name, moose_system, parameters),
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
