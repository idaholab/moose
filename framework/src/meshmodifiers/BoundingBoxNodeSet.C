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

#include "BoundingBoxNodeSet.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<BoundingBoxNodeSet>()
{
  MooseEnum location("INSIDE OUTSIDE", "INSIDE");

  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<std::vector<BoundaryName>>("new_boundary",
                                                     "The name of the nodeset to create");
  params.addRequiredParam<RealVectorValue>(
      "bottom_left",
      "The bottom left point (in x,y,z with spaces in-between) of the box to select the nodes.");
  params.addRequiredParam<RealVectorValue>(
      "top_right",
      "The bottom left point (in x,y,z with spaces in-between) of the box to select the nodes.");
  params.addParam<MooseEnum>("location", location, "Control of where the nodeset is to be set");

  return params;
}

BoundingBoxNodeSet::BoundingBoxNodeSet(const InputParameters & parameters)
  : MeshModifier(parameters),
    _location(getParam<MooseEnum>("location")),
    _bounding_box(getParam<RealVectorValue>("bottom_left"), getParam<RealVectorValue>("top_right"))
{
}

void
BoundingBoxNodeSet::modify()
{
  // Get the BoundaryIDs from the mesh
  std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName>>("new_boundary");
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(boundary_names, true);
  if (boundary_ids.size() != 1)
    mooseError("Only one boundary ID can be assigned to a nodeset using a bounding box!");

  // Get a reference to our BoundaryInfo object
  BoundaryInfo & boundary_info = _mesh_ptr->getMesh().get_boundary_info();

  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling BoundingBoxNodeSet::modify()");

  // Reference the the libMesh::MeshBase
  MeshBase & mesh = _mesh_ptr->getMesh();

  bool found_node = false;
  const bool inside = (_location == "INSIDE");

  // Loop over the elements and assign node set id to nodes within the bounding box
  for (auto node = mesh.active_nodes_begin(); node != mesh.active_nodes_end(); ++node)
  {
    if (_bounding_box.contains_point(**node) == inside)
    {
      boundary_info.add_node(*node, boundary_ids[0]);
      found_node = true;
    }
  }

  if (!found_node)
    mooseError("No nodes found within the bounding box");

  boundary_info.nodeset_name(boundary_ids[0]) = boundary_names[0];
}
