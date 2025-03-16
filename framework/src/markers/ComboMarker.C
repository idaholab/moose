//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComboMarker.h"
#include "FEProblemBase.h"

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
  : Marker(parameters),
    _names(parameters.get<std::vector<MarkerName>>("markers")),
    _block_restriction_mismatch(false)
{
  for (const auto & marker_name : _names)
    _markers.push_back(&getMarkerValue(marker_name));

  for (const auto & marker_name : _names)
    _marker_variables.push_back(&_fe_problem.getVariable(_tid, marker_name));

  // Check block restrictions
  std::string other_block_restricted = "";
  for (const auto & marker_name : _names)
    if (blockIDs() != _fe_problem.getVariable(_tid, marker_name).blockIDs())
      other_block_restricted += (other_block_restricted == "" ? "" : ", ") + marker_name;

  if (other_block_restricted != "")
  {
    _block_restriction_mismatch = true;
    paramInfo(
        "markers",
        "Combo marker and markers '" + other_block_restricted +
            "' do not share the same block restrictions. Markers outside their block restriction "
            "will not mark.");
  }
}

Marker::MarkerValue
ComboMarker::computeElementMarker()
{
  // We start with DONT_MARK because it's -1
  MarkerValue marker_value = DONT_MARK;

  // No need to check block restrictions if they all match
  if (!_block_restriction_mismatch)
    for (const auto & var : _markers)
      marker_value = std::max(marker_value, static_cast<MarkerValue>((*var)[0]));
  else
    for (const auto i : index_range(_markers))
      if (_marker_variables[i]->hasBlocks(_current_elem->subdomain_id()))
        marker_value = std::max(marker_value, static_cast<MarkerValue>((*_markers[i])[0]));

  return marker_value;
}
