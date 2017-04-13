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

#include "SubdomainBoundingBox.h"
#include "Conversion.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<SubdomainBoundingBox>()
{
  MooseEnum location("INSIDE OUTSIDE", "INSIDE");

  InputParameters params = validParams<MeshModifier>();
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

  return params;
}

SubdomainBoundingBox::SubdomainBoundingBox(const InputParameters & parameters)
  : MeshModifier(parameters),
    _location(parameters.get<MooseEnum>("location")),
    _block_id(parameters.get<SubdomainID>("block_id")),
    _bounding_box(parameters.get<RealVectorValue>("bottom_left"),
                  parameters.get<RealVectorValue>("top_right"))
{
}

void
SubdomainBoundingBox::modify()
{
  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling SubdomainBoundingBox::modify()");

  // Reference the the libMesh::MeshBase
  MeshBase & mesh = _mesh_ptr->getMesh();

  // Loop over the elements
  for (MeshBase::element_iterator el = mesh.active_elements_begin();
       el != mesh.active_elements_end();
       ++el)
  {
    bool contains = _bounding_box.contains_point((*el)->centroid());
    if (contains && _location == "INSIDE")
      (*el)->subdomain_id() = _block_id;
    else if (!contains && _location == "OUTSIDE")
      (*el)->subdomain_id() = _block_id;
  }

  // Assign block name, if provided
  if (isParamValid("block_name"))
    _mesh_ptr->getMesh().subdomain_name(_block_id) = getParam<SubdomainName>("block_name");
}
