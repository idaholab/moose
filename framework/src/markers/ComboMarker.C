//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComboMarker.h"

registerMooseObject("MooseApp", ComboMarker);

InputParameters
ComboMarker::validParams()
{
  InputParameters params = Marker::validParams();
  params.addRequiredParam<std::vector<MarkerName>>(
      "markers", "A list of marker names to combine into a single marker.");
  params.addClassDescription("A marker that converts many markers into a single marker by "
                             "considering the maximum value of the listed markers (i.e., "
                             "refinement takes precedent).");
  return params;
}

ComboMarker::ComboMarker(const InputParameters & parameters)
  : Marker(parameters), _names(parameters.get<std::vector<MarkerName>>("markers"))
{
  for (const auto & marker_name : _names)
    _markers.push_back(&getMarkerValue(marker_name));
}

Marker::MarkerValue
ComboMarker::computeElementMarker()
{
  // We start with DONT_MARK because it's -1
  MarkerValue marker_value = DONT_MARK;

  for (const auto & var : _markers)
    marker_value = std::max(marker_value, static_cast<MarkerValue>((*var)[0]));

  return marker_value;
}
