#include "SideAverageValue.h"

template<>
InputParameters validParams<SideAverageValue>()
{
  InputParameters params = validParams<SideIntegral>();
  return params;
}

SideAverageValue::SideAverageValue(const std::string & name, InputParameters parameters) :
    SideIntegral(name, parameters),
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
  // FIXME: current_side_elem
//  _volume += _current_side_elem->volume();
}

Real
SideAverageValue::getValue()
{
  Real integral = SideIntegral::getValue();
  
  gatherSum(_volume);
  
  return integral / _volume;
}
