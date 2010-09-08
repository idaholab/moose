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
  InputParameters params = validParams<SideIntegral>();
  return params;
}

SideAverageValue::SideAverageValue(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
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

  _volume += _current_side_elem->volume();
}

Real
SideAverageValue::getValue()
{
  Real integral = SideIntegral::getValue();
  
  gatherSum(_volume);
  
  return integral / _volume;
}
