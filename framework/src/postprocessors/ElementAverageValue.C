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

#include "ElementAverageValue.h"

template<>
InputParameters validParams<ElementAverageValue>()
{
  InputParameters params = validParams<ElementIntegral>();
  return params;
}

ElementAverageValue::ElementAverageValue(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :ElementIntegral(name, moose_system, parameters),
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
