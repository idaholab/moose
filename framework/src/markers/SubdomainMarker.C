//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainMarker.h"

registerMooseObject("MooseApp", SubdomainMarker);

InputParameters
SubdomainMarker::validParams()
{
  InputParameters params = Marker::validParams();
  params.addRequiredParam<std::vector<SubdomainName>>("block",
                                                      "The subdomain this marker is to be marked.");

  MooseEnum marker_states = Marker::markerStates();

  params.addRequiredParam<MooseEnum>(
      "inside", marker_states, "How to mark elements inside the subdomain.");
  params.addRequiredParam<MooseEnum>(
      "outside", marker_states, "How to mark elements outside the subdomain.");

  params.addClassDescription(
      "Marks the region inside and outside of a subdomain for refinement or coarsening.");
  return params;
}

SubdomainMarker::SubdomainMarker(const InputParameters & parameters)
  : Marker(parameters),
    _inside(parameters.get<MooseEnum>("inside").getEnum<MarkerValue>()),
    _outside(parameters.get<MooseEnum>("outside").getEnum<MarkerValue>()),
    _blocks(getParam<std::vector<SubdomainName>>("block"))
{
  auto blocks = _mesh.getSubdomainIDs(_blocks);
  for (const auto & block : blocks)
    _blk_ids.insert(block);
}

Marker::MarkerValue
SubdomainMarker::computeElementMarker()
{
  auto block_id = _current_elem->subdomain_id();

  if (_blk_ids.count(block_id))
    return _inside;

  return _outside;
}
