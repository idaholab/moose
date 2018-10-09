//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainBoundingBox.h"
#include "Conversion.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", SubdomainBoundingBox);

template <>
InputParameters
validParams<SubdomainBoundingBox>()
{
  MooseEnum location("INSIDE OUTSIDE", "INSIDE");

  InputParameters params = validParams<MeshModifier>();
  params.addClassDescription("Changes the subdomain ID of elements either (XOR) inside or outside "
                             "the specified box to the specified ID.");
  params.addRequiredParam<RealVectorValue>(
      "bottom_left", "The bottom left point (in x,y,z with spaces in-between).");
  params.addRequiredParam<RealVectorValue>(
      "top_right", "The bottom left point (in x,y,z with spaces in-between).");
  params.addRequiredParam<SubdomainID>("block_id",
                                       "Subdomain id to set for inside/outside the bounding box");
  params.addParam<SubdomainName>(
      "block_name", "Subdomain name to set for inside/outside the bounding box (optional)");
  params.addParam<MooseEnum>(
      "location", location, "Control of where the subdomain id is to be set");
  params.addParam<std::vector<SubdomainName>>(
      "restricted_subdomains",
      "Only reset subdomain ID for given subdomains within the bounding box");

  return params;
}

SubdomainBoundingBox::SubdomainBoundingBox(const InputParameters & parameters)
  : MeshModifier(parameters),
    _location(parameters.get<MooseEnum>("location")),
    _block_id(parameters.get<SubdomainID>("block_id")),
    _bounding_box(parameters.get<RealVectorValue>("bottom_left"),
                  parameters.get<RealVectorValue>("top_right")),
    _restricted(isParamValid("restricted_subdomains"))
{
}

void
SubdomainBoundingBox::modify()
{
  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling SubdomainBoundingBox::modify()");

  std::set<SubdomainID> restricted_ids;
  if (_restricted)
  {
    auto names = getParam<std::vector<SubdomainName>>("restricted_subdomains");
    auto ids = _mesh_ptr->getSubdomainIDs(names);
    for (auto & id : ids)
      restricted_ids.insert(id);
  }

  // Loop over the elements
  for (const auto & elem : _mesh_ptr->getMesh().active_element_ptr_range())
  {
    bool contains = _bounding_box.contains_point(elem->centroid());
    bool restricted = _restricted ? restricted_ids.count(elem->subdomain_id()) > 0 : true;
    if (contains && restricted && _location == "INSIDE")
      elem->subdomain_id() = _block_id;
    else if (!contains && restricted && _location == "OUTSIDE")
      elem->subdomain_id() = _block_id;
  }

  // Assign block name, if provided
  if (isParamValid("block_name"))
    _mesh_ptr->getMesh().subdomain_name(_block_id) = getParam<SubdomainName>("block_name");
}
