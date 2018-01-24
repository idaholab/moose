//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddSideSetsFromBoundingBox.h"
#include "Conversion.h"
#include "MooseMesh.h"
#include "MooseTypes.h"

template <>
InputParameters
validParams<AddSideSetsFromBoundingBox>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addClassDescription("Find sidesets with given boundary ids in bounding box and add new "
                             "boundary id. This can be done by finding all required boundary "
                             "and adding the new boundary id to those sidesets. Alternatively, "
                             "a number of boundary ids can be provided and all nodes within the "
                             "bounding box that have all the required boundary ids will have a new"
                             "boundary id added.");

  MooseEnum location("INSIDE OUTSIDE", "INSIDE");

  params.addRequiredParam<RealVectorValue>(
      "bottom_left", "The bottom left point (in x,y,z with spaces in-between).");
  params.addRequiredParam<RealVectorValue>(
      "top_right", "The bottom left point (in x,y,z with spaces in-between).");
  params.addRequiredParam<SubdomainID>("block_id",
                                       "Subdomain id to set for inside/outside the bounding box");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary_id_old", "Boundary id on specified block within the bounding box to select");
  params.addRequiredParam<boundary_id_type>(
      "boundary_id_new", "Boundary id on specified block within the bounding box to assign");
  params.addParam<bool>("boundary_id_overlap",
                        false,
                        "Set to true if boundaries need to overlap on sideset to be detected.");
  params.addParam<MooseEnum>(
      "location", location, "Control of where the subdomain id is to be set");

  return params;
}

AddSideSetsFromBoundingBox::AddSideSetsFromBoundingBox(const InputParameters & parameters)
  : MeshModifier(parameters),
    _location(parameters.get<MooseEnum>("location")),
    _block_id(parameters.get<SubdomainID>("block_id")),
    _boundary_id_old(parameters.get<std::vector<BoundaryName>>("boundary_id_old")),
    _boundary_id_new(parameters.get<boundary_id_type>("boundary_id_new")),
    _bounding_box(parameters.get<RealVectorValue>("bottom_left"),
                  parameters.get<RealVectorValue>("top_right")),
    _boundary_id_overlap(parameters.get<bool>("boundary_id_overlap"))
{
}

void
AddSideSetsFromBoundingBox::modify()
{
  // this modifier is not designed for working with distributed mesh
  _mesh_ptr->errorIfDistributedMesh("BreakBoundaryOnSubdomain");

  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling SubdomainBoundingBox::modify()");

  // Reference the the libMesh::MeshBase
  MeshBase & mesh = _mesh_ptr->getMesh();

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  bool found_element = false;
  bool found_side_sets = false;

  if (!_boundary_id_overlap)
  {
    // Loop over the elements
    for (const auto & elem : mesh.active_element_ptr_range())
    {
      // boolean if element centroid is in bounding box
      bool contains = _bounding_box.contains_point(elem->centroid());

      // check if active elements are found in the bounding box
      if (contains)
      {
        found_element = true;
        // loop over sides of elements within bounding box
        for (unsigned int side = 0; side < elem->n_sides(); side++)
          // loop over provided boundary vector to check all side sets for all boundary ids
          for (unsigned int boundary_id_number = 0; boundary_id_number < _boundary_id_old.size();
               boundary_id_number++)
            // check if side has same boundary id that you are looking for
            if (boundary_info.has_boundary_id(
                    elem, side, boundary_info.get_id_by_name(_boundary_id_old[boundary_id_number])))
            {
              // assign new boundary value to boundary which meets meshmodifier criteria
              boundary_info.add_side(elem, side, _boundary_id_new);
              found_side_sets = true;
            }
      }
    }
    if (!found_element)
      mooseError("No elements found within the bounding box");

    if (!found_side_sets)
      mooseError("No side sets found on active elements within the bounding box");
  }

  else if (_boundary_id_overlap)
  {
    if (_boundary_id_old.size() < 2)
      mooseError("boundary_id_old out of bounds: ",
                 _boundary_id_old.size(),
                 " Must be 2 boundary inputs or more.");

    bool found_node = false;
    const bool inside = (_location == "INSIDE");

    // Loop over the elements and assign node set id to nodes within the bounding box
    for (auto node = mesh.active_nodes_begin(); node != mesh.active_nodes_end(); ++node)
    {
      // check if nodes are inside of bounding box
      if (_bounding_box.contains_point(**node) == inside)
      {
        // read out boundary ids for nodes
        std::vector<short int> boundary_id_list = boundary_info.boundary_ids(*node);
        std::vector<boundary_id_type> boundary_id_old_list =
            _mesh_ptr->getBoundaryIDs(_boundary_id_old);

        // sort boundary ids on node and sort boundary ids provided in input file
        std::sort(boundary_id_list.begin(), boundary_id_list.end());
        std::sort(boundary_id_old_list.begin(), boundary_id_old_list.end());

        // check if input boundary ids are all contained in the node
        // if true, write new boundary id on respective node
        if (std::includes(boundary_id_list.begin(),
                          boundary_id_list.end(),
                          boundary_id_old_list.begin(),
                          boundary_id_old_list.end()))
        {
          boundary_info.add_node(*node, _boundary_id_new);
          found_node = true;
        }
      }
    }

    if (!found_node)
      mooseError("No nodes found within the bounding box");
  }
}
