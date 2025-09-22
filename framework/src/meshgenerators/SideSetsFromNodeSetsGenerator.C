//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetsFromNodeSetsGenerator.h"

#include "CastUniquePointer.h"
#include "libmesh/boundary_info.h"
#include "libmesh/elem_side_builder.h"

registerMooseObject("MooseApp", SideSetsFromNodeSetsGenerator);

InputParameters
SideSetsFromNodeSetsGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("Mesh generator which constructs side sets from node sets");
  params.addRequiredParam<MeshGeneratorName>("input",
                                             "Input mesh the operation will be applied to");
  params.addParam<std::vector<BoundaryName>>(
      "nodesets_to_convert",
      "If specified, list of nodesets to convert. If not specified, all nodesets are converted");
  return params;
}

SideSetsFromNodeSetsGenerator::SideSetsFromNodeSetsGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
SideSetsFromNodeSetsGenerator::generate()
{
  if (!isParamValid("nodesets_to_convert"))
    _input->get_boundary_info().build_side_list_from_node_list();
  else
  {
    const auto & nodeset_names = getParam<std::vector<BoundaryName>>("nodesets_to_convert");
    auto & binfo = _input->get_boundary_info();

    std::map<BoundaryName, BoundaryID> new_nodeset_ids;
    std::set<BoundaryID> nodeset_ids;

    for (const auto & nodeset_name : nodeset_names)
    {
      // Look through the nodeset map.
      BoundaryID nodeset_id = std::numeric_limits<BoundaryID>::max();
      for (const auto & [id, name] : binfo.get_nodeset_name_map())
        if (name == nodeset_name)
          nodeset_id = id;
      if (MooseUtils::isDigits(nodeset_name))
        nodeset_id = std::stoi(nodeset_name);
      if (nodeset_id == std::numeric_limits<BoundaryID>::max())
        paramError("nodesets_to_convert",
                   "Nodeset '" + nodeset_name + "' does not exist in the input mesh");
      nodeset_ids.insert(nodeset_id);

      // Find the target ID if there is already a sideset with that name
      if (MooseUtils::isDigits(nodeset_name))
        new_nodeset_ids[nodeset_name] = nodeset_id;
      else if (binfo.get_sideset_name(nodeset_id) == nodeset_name)
        new_nodeset_ids[nodeset_name] = binfo.get_id_by_name(nodeset_name);
      else
        new_nodeset_ids[nodeset_name] = nodeset_id;
    }

    // Copy pasted from libMesh's build_side_list_from_node_list
    // For avoiding extraneous element side construction
    libMesh::ElemSideBuilder side_builder;
    // Pull objects out of the loop to reduce heap operations
    const Elem * side_elem;
    const auto & boundary_node_ids = binfo.get_nodeset_map();

    for (const auto & elem : _input->active_element_ptr_range())
      for (auto side : elem->side_index_range())
      {
        side_elem = &side_builder(*elem, side);

        // map from nodeset_id to count for that ID
        std::map<boundary_id_type, unsigned> nodesets_node_count;

        // For each nodeset that this node is a member of, increment the associated
        // nodeset ID count
        for (const auto & node : side_elem->node_ref_range())
          for (const auto & pr : as_range(boundary_node_ids.equal_range(&node)))
            // Single added new line
            if (nodeset_ids.count(pr.second))
              nodesets_node_count[pr.second]++;

        // Now check to see what nodeset_counts have the correct
        // number of nodes in them.  For any that do, add this side to
        // the sideset, making sure the sideset inherits the
        // nodeset's name, if there is one.
        for (const auto & pr : nodesets_node_count)
          if (pr.second == side_elem->n_nodes())
            binfo.add_side(elem, side, pr.first);

        // Add nodeset name in case it does not exist yet
        for (const auto & [nodeset_name, new_nodeset_id] : new_nodeset_ids)
          binfo.sideset_name(new_nodeset_id) = nodeset_name;
      }
  }

  return dynamic_pointer_cast<MeshBase>(_input);
}
