#include "SideAverageValue.h"

template<>
InputParameters validParams<SideAverageValue>()
{
  InputParameters params = validParams<SideIntegral>();
  return params;
}

SideAverageValue::SideAverageValue(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :SideIntegral(name, moose_system, parameters),
   _volume(0)
{}

void
SideAverageValue::initialize()
{
  SideIntegral::initialize();
  
  _volume = 0;
}

void
SideAverageValue::execute()
{
  SideIntegral::execute();

  _volume += _current_elem->volume();
}

Real
SideAverageValue::getValue()
{
  Real integral = SideIntegral::getValue();
  
  gatherSum(_volume);
  
  return integral / _volume;
}
