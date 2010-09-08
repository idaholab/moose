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

#include "AverageElementSize.h"

template<>
InputParameters validParams<AverageElementSize>()
{
  InputParameters params = validParams<ElementAverageValue>();
  return params;
}

AverageElementSize::AverageElementSize(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :ElementAverageValue(name, moose_system, parameters)
{}

void
AverageElementSize::initialize()
{
  ElementAverageValue::initialize();
  _elems = 0;
}

void
AverageElementSize::execute()
{
  ElementIntegral::execute();
  _elems ++;
}

Real
AverageElementSize::computeIntegral()
{
  return _current_elem->hmax();
}

Real
AverageElementSize::getValue()
{
  Real integral = ElementIntegral::getValue();

  gatherSum(_elems);

  return integral / _elems;
}
