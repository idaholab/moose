//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OrientedSubdomainBoundingBox.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<OrientedSubdomainBoundingBox>()
{
  InputParameters params = validParams<MeshModifier>();
  params += validParams<OrientedBoxInterface>();

  MooseEnum location("INSIDE OUTSIDE", "INSIDE");
  params.addRequiredParam<SubdomainID>("block_id",
                                       "Subdomain id to set for inside/outside the bounding box");
  params.addParam<MooseEnum>(
      "location", location, "Control of where the subdomain id is to be set");

  return params;
}

OrientedSubdomainBoundingBox::OrientedSubdomainBoundingBox(const InputParameters & parameters)
  : MeshModifier(parameters),
    OrientedBoxInterface(parameters),
    _location(parameters.get<MooseEnum>("location")),
    _block_id(parameters.get<SubdomainID>("block_id"))

{
}

void
OrientedSubdomainBoundingBox::modify()
{
  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling SubdomainBoundingBox::modify()");

  // Loop over the elements
  for (const auto & elem : _mesh_ptr->getMesh().active_element_ptr_range())
  {
    bool contains = containsPoint(elem->centroid());
    if (contains && _location == "INSIDE")
      elem->subdomain_id() = _block_id;
    else if (!contains && _location == "OUTSIDE")
      elem->subdomain_id() = _block_id;
  }
}
