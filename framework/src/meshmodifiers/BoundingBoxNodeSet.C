//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundingBoxNodeSet.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<BoundingBoxNodeSet>()
{
  MooseEnum location("INSIDE OUTSIDE", "INSIDE");

  InputParameters params = validParams<MeshModifier>();
  params.addClassDescription(
      "Assigns all of the nodes either inside or outside of a bounding box to a new nodeset.");
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

  // Unless at least one processor found a node in the bounding box,
  // the user probably specified it incorrectly.
  this->comm().max(found_node);

  if (!found_node)
    mooseError("No nodes found within the bounding box");

  boundary_info.nodeset_name(boundary_ids[0]) = boundary_names[0];
}
