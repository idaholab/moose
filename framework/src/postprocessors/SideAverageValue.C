/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SideAverageValue.h"

template<>
InputParameters validParams<SideAverageValue>()
{
  InputParameters params = validParams<SideIntegralVariablePostprocessor>();
  return params;
}

SideAverageValue::SideAverageValue(const std::string & name, InputParameters parameters) :
    SideIntegralVariablePostprocessor(name, parameters),
    _volume(0)
{}

void
SideAverageValue::initialize()
{
  SideIntegralVariablePostprocessor::initialize();
  _volume = 0;
}

void
SideAverageValue::execute()
{
  SideIntegralVariablePostprocessor::execute();
  _volume += _current_side_volume;
}

Real
SideAverageValue::getValue()
{
  Real integral = SideIntegralVariablePostprocessor::getValue();

  gatherSum(_volume);

  return integral / _volume;
}


void
SideAverageValue::threadJoin(const UserObject & y)
{
  SideIntegralVariablePostprocessor::threadJoin(y);
  const SideAverageValue & pps = static_cast<const SideAverageValue &>(y);
  _volume += pps._volume;
}
