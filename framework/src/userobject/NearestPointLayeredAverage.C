//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NearestPointLayeredAverage.h"
#include "LayeredAverage.h"

template <>
InputParameters
validParams<NearestPointLayeredAverage>()
{
  InputParameters params = nearestPointBaseValidParams<LayeredAverage>();

  return params;
}

NearestPointLayeredAverage::NearestPointLayeredAverage(const InputParameters & parameters)
  : NearestPointBase<LayeredAverage>(parameters)
{
}
