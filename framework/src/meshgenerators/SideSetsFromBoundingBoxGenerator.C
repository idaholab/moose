//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetsFromBoundingBoxGenerator.h"
#include "Conversion.h"
#include "MooseTypes.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"
#include "MooseUtils.h"

#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"

#include <typeinfo>

registerMooseObject("MooseApp", SideSetsFromBoundingBoxGenerator);

InputParameters
SideSetsFromBoundingBoxGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Defines new sidesets using currently-defined sideset IDs inside or "
                             "outside of a bounding box.");

  MooseEnum location("INSIDE OUTSIDE", "INSIDE");

  params.addRequiredParam<RealVectorValue>(
      "bottom_left", "The bottom left point (in x,y,z with spaces in-between).");
  params.addRequiredParam<RealVectorValue>(
      "top_right", "The bottom left point (in x,y,z with spaces in-between).");
  params.addRequiredParam<subdomain_id_type>(
      "block_id", "Subdomain id to set for inside/outside the bounding box");
  params.addDeprecatedParam<std::vector<BoundaryName>>(
      "boundary_id_old",
      "Boundary id on specified block within the bounding box to select",
      "boundary_id_old is deprecated, use boundaries_old with names or ids");
  params.addDeprecatedParam<BoundaryName>(
      "boundary_id_new",
      "Boundary id on specified block within the bounding box to assign",
      "boundary_id_new is deprecated, use boundary_new with a name or id");
  params.addParam<std::vector<BoundaryName>>(
      "boundaries_old",
      "The list of boundaries on the specified block within the bounding box to be modified");
  params.addParam<BoundaryName>("boundary_new",
                                "Boundary on specified block within the bounding box to assign");
  params.addParam<bool>("boundary_id_overlap",
                        false,
                        "Set to true if boundaries need to overlap on sideset to be detected.");
  params.addParam<MooseEnum>(
      "location", location, "Control of where the subdomain id is to be set");

  return params;
}

SideSetsFromBoundingBoxGenerator::SideSetsFromBoundingBoxGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _location(parameters.get<MooseEnum>("location")),
    _block_id(parameters.get<SubdomainID>("block_id")),
    _bounding_box(MooseUtils::buildBoundingBox(parameters.get<RealVectorValue>("bottom_left"),
                                               parameters.get<RealVectorValue>("top_right"))),
    _boundary_id_overlap(parameters.get<bool>("boundary_id_overlap"))
{
  if (parameters.isParamSetByUser("boundaries_old"))
    _boundaries_old = parameters.get<std::vector<BoundaryName>>("boundaries_old");
  else if (parameters.isParamSetByUser("boundary_id_old"))
    _boundaries_old = parameters.get<std::vector<BoundaryName>>("boundary_id_old");
  else
    mooseError("Either boundaries_old or boundary_id_old (deprecated) must be specified.");

  if (parameters.isParamSetByUser("boundary_new"))
    _boundary_new = parameters.get<BoundaryName>("boundary_new");
  else if (parameters.isParamSetByUser("boundary_id_new"))
    _boundary_new = parameters.get<BoundaryName>("boundary_id_new");
  else
    mooseError("Either boundary_new or boundary_id_new (deprecated) must be specified.");
}

std::unique_ptr<MeshBase>
SideSetsFromBoundingBoxGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  if (!mesh->is_replicated())
    mooseError("SideSetsFromBoundingBoxGenerator is not implemented for distributed meshes");

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  boundary_info.build_node_list_from_side_list();

  bool found_element = false;
  bool found_side_sets = false;
  const bool inside = (_location == "INSIDE");

  // Get the list of boundary ids from the boundary names
  auto boundary_ids = MooseMeshUtils::getBoundaryIDs(*mesh, _boundaries_old, false);

  // Check that the old boundary ids/names exist in the mesh
  for (std::size_t i = 0; i < boundary_ids.size(); ++i)
    if (boundary_ids[i] == Moose::INVALID_BOUNDARY_ID)
      paramError(
          "boundaries", "The boundary '", _boundaries_old[i], "' was not found within the mesh");

  // Attempt to get the new boundary id from the name
  auto boundary_id_new = MooseMeshUtils::getBoundaryID(_boundary_new, *mesh);

  // If the new boundary id is not valid, make it instead
  if (boundary_id_new == Moose::INVALID_BOUNDARY_ID)
  {
    boundary_id_new = MooseMeshUtils::getNextFreeBoundaryID(*mesh);

    // Write the name alias of the boundary id to the mesh boundary info
    boundary_info.sideset_name(boundary_id_new) = _boundary_new;
    boundary_info.nodeset_name(boundary_id_new) = _boundary_new;
  }

  if (!_boundary_id_overlap)
  {
    // Loop over the elements
    for (const auto & elem : mesh->active_element_ptr_range())
    {
      // boolean if element centroid is in bounding box
      bool contains = _bounding_box.contains_point(elem->vertex_average());

      // check if active elements are found either in or out of the bounding box, apropos "inside"
      if (contains == inside)
      {
        found_element = true;
        // loop over sides of elements within bounding box
        for (unsigned int side = 0; side < elem->n_sides(); side++)
          // loop over provided boundary vector to check all side sets for all boundary ids
          for (auto boundary_id : boundary_ids)
            // check if side has same boundary id that you are looking for
            if (boundary_info.has_boundary_id(elem, side, boundary_id))
            {
              // assign new boundary value to boundary which meets meshmodifier criteria
              boundary_info.add_side(elem, side, boundary_id_new);
              found_side_sets = true;
            }
      }
    }
    if (!found_element && inside)
      mooseError("No elements found within the bounding box");

    if (!found_element && !inside)
      mooseError("No elements found outside the bounding box");

    if (!found_side_sets)
      mooseError("No side sets found on active elements within the bounding box");
  }

  else if (_boundary_id_overlap)
  {
    if (boundary_ids.size() < 2)
      mooseError("boundary_id_old out of bounds: ",
                 boundary_ids.size(),
                 " Must be 2 boundary inputs or more.");

    bool found_node = false;

    // Loop over the elements and assign node set id to nodes within the bounding box
    for (auto node = mesh->active_nodes_begin(); node != mesh->active_nodes_end(); ++node)
    {
      // check if nodes are inside of bounding box
      if (_bounding_box.contains_point(**node) == inside)
      {
        // read out boundary ids for nodes
        std::vector<boundary_id_type> node_boundary_ids;
        boundary_info.boundary_ids(*node, node_boundary_ids);

        // sort boundary ids on node and sort boundary ids provided in input file
        std::sort(node_boundary_ids.begin(), node_boundary_ids.end());
        std::sort(boundary_ids.begin(), boundary_ids.end());

        // check if input boundary ids are all contained in the node
        // if true, write new boundary id on respective node
        if (std::includes(node_boundary_ids.begin(),
                          node_boundary_ids.end(),
                          boundary_ids.begin(),
                          boundary_ids.end()))
        {
          boundary_info.add_node(*node, boundary_id_new);
          found_node = true;
        }
      }
    }

    if (!found_node)
      mooseError("No nodes found within the bounding box");
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}
