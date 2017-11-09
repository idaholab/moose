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
