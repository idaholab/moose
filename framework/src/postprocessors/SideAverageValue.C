//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideAverageValue.h"

registerMooseObject("MooseApp", SideAverageValue);

InputParameters
SideAverageValue::validParams()
{
  InputParameters params = SideIntegralVariablePostprocessor::validParams();
  params.addClassDescription("Computes the average value of a variable on a "
                             "sideset. Note that this cannot be used on the "
                             "centerline of an axisymmetric model.");
  return params;
}

SideAverageValue::SideAverageValue(const InputParameters & parameters)
  : SideIntegralVariablePostprocessor(parameters), _volume(0)
{
}

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
  _volume += volume();
}

Real
SideAverageValue::getValue()
{
  if (MooseUtils::absoluteFuzzyEqual(_volume, 0.0))
  {
    if (_coord_sys == Moose::COORD_RZ)
      mooseError("The total area of the boundary is zero. This could be due to "
                 "using a boundary on the centerline of an axisymmetric model.");
    else
      mooseError("The total area of the boundary is zero.");
  }

  return _integral_value / _volume;
}

Real
SideAverageValue::volume()
{
  return _current_side_volume;
}

void
SideAverageValue::threadJoin(const UserObject & y)
{
  SideIntegralVariablePostprocessor::threadJoin(y);
  const SideAverageValue & pps = static_cast<const SideAverageValue &>(y);
  _volume += pps._volume;
}

void
SideAverageValue::finalize()
{
  gatherSum(_volume);
  gatherSum(_integral_value);
}
