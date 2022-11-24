//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AverageElementSize.h"

registerMooseObject("MooseApp", AverageElementSize);

InputParameters
AverageElementSize::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();
  params.addClassDescription("Computes the average element size.");
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

  return _total_size / _elems;
}

void
AverageElementSize::threadJoin(const UserObject & y)
{
  const AverageElementSize & pps = static_cast<const AverageElementSize &>(y);
  _total_size += pps._total_size;
  _elems += pps._elems;
}

void
AverageElementSize::finalize()
{
  gatherSum(_total_size);
  gatherSum(_elems);
}
