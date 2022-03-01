//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundingBoxNodeSetGenerator.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"
#include "MooseUtils.h"

#include "libmesh/node.h"

registerMooseObject("MooseApp", BoundingBoxNodeSetGenerator);

InputParameters
BoundingBoxNodeSetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  MooseEnum location("INSIDE OUTSIDE", "INSIDE");

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
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

BoundingBoxNodeSetGenerator::BoundingBoxNodeSetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _location(getParam<MooseEnum>("location")),
    _bounding_box(MooseUtils::buildBoundingBox(getParam<RealVectorValue>("bottom_left"),
                                               getParam<RealVectorValue>("top_right")))
{
}

std::unique_ptr<MeshBase>
BoundingBoxNodeSetGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Get the BoundaryIDs from the mesh
  std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName>>("new_boundary");
  std::vector<BoundaryID> boundary_ids =
      MooseMeshUtils::getBoundaryIDs(*mesh, boundary_names, true);
  if (boundary_ids.size() != 1)
    mooseError("Only one boundary ID can be assigned to a nodeset using a bounding box!");

  // Get a reference to our BoundaryInfo object
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  bool found_node = false;
  const bool inside = (_location == "INSIDE");

  // Loop over the elements and assign node set id to nodes within the bounding box
  for (auto & node : as_range(mesh->active_nodes_begin(), mesh->active_nodes_end()))
    if (_bounding_box.contains_point(*node) == inside)
    {
      boundary_info.add_node(node, boundary_ids[0]);
      found_node = true;
    }

  // Unless at least one processor found a node in the bounding box,
  // the user probably specified it incorrectly.
  this->comm().max(found_node);

  if (!found_node)
    mooseError("No nodes found within the bounding box");

  boundary_info.nodeset_name(boundary_ids[0]) = boundary_names[0];

  return dynamic_pointer_cast<MeshBase>(mesh);
}
