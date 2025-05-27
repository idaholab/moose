//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryLayerSubdomainGenerator.h"
#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", BoundaryLayerSubdomainGenerator);

InputParameters
BoundaryLayerSubdomainGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription(
      "Changes the subdomain ID of elements near the specified boundary(ies).");

  params.addRequiredParam<SubdomainName>(
      "block_name", "Subdomain name to set for inside/outside the bounding box (optional)");
  params.addParam<subdomain_id_type>("block_id",
                                     "Subdomain id to set for inside/outside the bounding box");

  params.addParam<bool>("include_nodesets",
                        false,
                        "Whether to include nodesets in the boundaries. Nodesets are not sided so "
                        "elements on both sides of the nodesets will be included");
  params.addRequiredParam<std::vector<BoundaryName>>("boundaries",
                                                     "Boundaries to add the layer next to");
  return params;
}

BoundaryLayerSubdomainGenerator::BoundaryLayerSubdomainGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _new_block_name(getParam<SubdomainName>("block_name")),
    _include_nodesets(getParam<bool>("include_nodesets"))
{
}

std::unique_ptr<MeshBase>
BoundaryLayerSubdomainGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Get the next free block id
  _new_block_id = MooseMeshUtils::getNextFreeSubdomainID(*mesh);

  // Get the ids for the boundaries
  const auto boundary_ids = MooseMeshUtils::getBoundaryIDs(
      *mesh, getParam<std::vector<BoundaryName>>("boundaries"), false);

  // Boundary info
  const auto & boundary_info = mesh->get_boundary_info();

  // Loop over the elements
  for (const auto & elem : mesh->element_ptr_range())
  {
    // Check all the sides for a boundary in the concerned list
    bool next_to_a_boundary = false;
    for (const auto side : elem->side_index_range())
      for (const auto bid : boundary_ids)
        if (boundary_info.has_boundary_id(elem, side, bid))
        {
          next_to_a_boundary = true;
          goto out_loop_1;
        }
  out_loop_1:;

    // Check all the nodes in case the boundary is a nodeset
    if (!next_to_a_boundary && _include_nodesets)
    {
      for (const auto node_index : elem->node_index_range())
        for (const auto bid : boundary_ids)
          if (boundary_info.has_boundary_id(elem->node_ptr(node_index), bid))
          {
            next_to_a_boundary = true;
            goto out_loop_2;
          }
    out_loop_2:;
    }

    if (next_to_a_boundary)
      elem->subdomain_id() = _new_block_id;
  }

  // Assign block name
  mesh->subdomain_name(_new_block_id) = _new_block_name;

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
