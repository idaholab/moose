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

template <>
InputParameters
validParams<AverageElementSize>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  return params;
}

AverageElementSize::AverageElementSize(const InputParameters & parameters)
  : ElementPostprocessor(parameters)
{
}

void
AverageElementSize::initialize()
{
  _total_size = 0;
  _elems = 0;
}

void
AverageElementSize::execute()
{
  _total_size += _current_elem->hmax();
  _elems++;
}

Real
AverageElementSize::getValue()
{
  gatherSum(_total_size);
  gatherSum(_elems);

  return _total_size / _elems;
}

void
AverageElementSize::threadJoin(const UserObject & y)
{
  const AverageElementSize & pps = static_cast<const AverageElementSize &>(y);
  _total_size += pps._total_size;
  _elems += pps._elems;
}
