//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IndicatorMarker.h"

InputParameters
IndicatorMarker::validParams()
{
  InputParameters params = Marker::validParams();
  params.addRequiredParam<IndicatorName>("indicator",
                                         "The name of the Indicator that this Marker uses.");
  return params;
}

IndicatorMarker::IndicatorMarker(const InputParameters & parameters)
  : Marker(parameters), _error_vector(getErrorVector(parameters.get<IndicatorName>("indicator")))
{
  if (!_subproblem.getVariable(0, getParam<IndicatorName>("indicator")).hasBlocks(blockIDs()))
    mooseWarning("The block restriction of the marker is larger than the block restriction of its "
                 "indicator. Blocks on which the indicator is not active will not be marked, and "
                 "therefore will not be refined or coarsened");
}
