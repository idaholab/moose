//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshRepairGenerator.h"
#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/mesh_modification.h"

registerMooseObject("MooseApp", MeshRepairGenerator);

InputParameters
MeshRepairGenerator::validParams()
{

  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Mesh generator to perform various improvement / fixing operations on an input mesh");
  params.addRequiredParam<MeshGeneratorName>("input",
                                             "Name of the mesh generator providing the mesh");

  params.addParam<bool>("fix_node_overlap", false, "Whether to merge overlapping nodes");
  params.addParam<Real>(
      "node_overlap_tol", 1e-8, "Absolute tolerance for merging overlapping nodes");

  params.addParam<bool>(
      "fix_elements_orientation", false, "Whether to flip elements with negative volumes");

  params.addParam<bool>("separate_blocks_by_element_types",
                        false,
                        "Create new blocks if multiple element types are present in a block");

  params.addParam<bool>("remove_small_elements",
                        false,
                        "Whether to remove all small elements below a certain absolute volume "
                        "threshold and stitch up the mesh");
  params.addParam<Real>(
      "volume_threshold", 1e-8, "Minimum absolute size below which to remove elements");
  params.addParam<bool>(
      "absolute_volumes",
      true,
      "Whether to consider absolute volumes or not when examining volumes for element deletion");
  return params;
}

MeshRepairGenerator::MeshRepairGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _fix_overlapping_nodes(getParam<bool>("fix_node_overlap")),
    _node_overlap_tol(getParam<Real>("node_overlap_tol")),
    _fix_element_orientation(getParam<bool>("fix_elements_orientation")),
    _elem_type_separation(getParam<bool>("separate_blocks_by_element_types")),
    _remove_small_volumes(getParam<bool>("remove_small_elements")),
    _min_volume_threshold(getParam<Real>("volume_threshold")),
    _absolute_volumes(getParam<bool>("absolute_volumes"))
{
  if (!_fix_overlapping_nodes && !_fix_element_orientation && !_elem_type_separation &&
      !_remove_small_volumes)
    mooseError("MeshRepairGenerator has not been specified any item to fix");
  if (!isParamSetByUser("remove_small_elements") &&
      (isParamSetByUser("volume_threshold") || isParamSetByUser("absolute_volumes")))
    paramError("remove_small_elements",
               "Some parameters were passed for a repair operation that was not requested");
}

std::unique_ptr<MeshBase>
MeshRepairGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  mesh->prepare_for_use();

  // Blanket ban on distributed. This can be relaxed for some operations if needed
  if (!mesh->is_serial())
    mooseError("MeshRepairGenerator requires a serial mesh. The mesh should not be distributed.");

  if (_remove_small_volumes)
    removeSmallVolumeElements(mesh);

  if (_fix_overlapping_nodes)
    fixOverlappingNodes(mesh);

  // Flip orientation of elements to keep positive volumes
  if (_fix_element_orientation)
    MeshTools::Modification::orient_elements(*mesh);

  // Disambiguate any block that has elements of multiple types
  if (_elem_type_separation)
  {
    std::set<subdomain_id_type> ids;
    mesh->subdomain_ids(ids);
    // loop on sub-domain
    for (auto & id : ids)
    {
      // Gather all the element types and blocks
      // ElemType defines an enum for geometric element types
      std::set<ElemType> types;
      // loop on elements within this sub-domain
      for (auto & elem : mesh->active_subdomain_elements_ptr_range(id))
        types.insert(elem->type());

      if (types.size() > 1)
      {
        auto next_block_id = MooseMeshUtils::getNextFreeSubdomainID(*mesh);
        subdomain_id_type i = 0;
        for (const auto type : types)
        {
          auto new_id = next_block_id + i++;
          // Create blocks when a block has multiple element types
          mesh->subdomain_name(new_id) = mesh->subdomain_name(id) + "_" + Moose::stringify(type);

          // Re-assign elements to the new blocks
          for (auto elem : mesh->active_subdomain_elements_ptr_range(id))
            if (elem->type() == type)
              elem->subdomain_id() = new_id;
        }
      }
    }
  }

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
MeshRepairGenerator::removeSmallVolumeElements(std::unique_ptr<MeshBase> & mesh) const
{
  unsigned int num_elements_removed = 0;
  for (auto & elem : mesh->element_ptr_range())
  {
    if ((_absolute_volumes && std::abs(elem->volume()) < _min_volume_threshold) ||
        (!_absolute_volumes && elem->volume() < _min_volume_threshold))
    {
      num_elements_removed++;
      // Move all nodes to the centroid
      const Point centroid = elem->true_centroid();
      for (auto & node : elem->node_ref_range())
        node.subtract(node - centroid);

      mesh->delete_elem(elem);
    }
  }
  _console << "Number of small positive volume elements that were deleted: " << num_elements_removed
           << std::endl;

  mesh->contract();
}

void
MeshRepairGenerator::fixOverlappingNodes(std::unique_ptr<MeshBase> & mesh) const
{
  unsigned int num_fixed_nodes = 0;
  auto pl = mesh->sub_point_locator();
  std::unordered_set<dof_id_type> nodes_removed;
  // loop on nodes
  for (auto & node : mesh->node_ptr_range())
  {
    // this node has already been removed
    if (nodes_removed.count(node->id()))
      continue;

    // find all the elements around this node
    std::set<const Elem *> elements;
    (*pl)(*node, elements);

    for (auto & elem : elements)
    {
      bool found = false;
      for (auto & elem_node : elem->node_ref_range())
      {
        if (node->id() == elem_node.id())
        {
          found = true;
          break;
        }
      }
      if (!found)
      {
        for (auto & elem_node : elem->node_ref_range())
        {
          const Real tol = _node_overlap_tol;
          // Compares the coordinates
          const auto x_node = (*node)(0);
          const auto x_elem_node = elem_node(0);
          const auto y_node = (*node)(1);
          const auto y_elem_node = elem_node(1);
          const auto z_node = (*node)(2);
          const auto z_elem_node = elem_node(2);

          if (MooseUtils::absoluteFuzzyEqual(x_node, x_elem_node, tol) &&
              MooseUtils::absoluteFuzzyEqual(y_node, y_elem_node, tol) &&
              MooseUtils::absoluteFuzzyEqual(z_node, z_elem_node, tol))
          {
            // Coordinates are the same but it's not the same node
            // Replace the node in the element
            const_cast<Elem *>(elem)->set_node(elem->get_node_index(&elem_node)) = node;
            nodes_removed.insert(elem_node.id());

            num_fixed_nodes++;
            if (num_fixed_nodes < 10)
              _console << "Stitching a node at : " << *node << std::endl;
            else if (num_fixed_nodes == 10)
              _console << "Node stitching will now proceed silently." << std::endl;
          }
        }
      }
    }
  }
  _console << "Number of overlapping nodes which got merged: " << num_fixed_nodes << std::endl;
  if (mesh->allow_renumbering())
    mesh->renumber_nodes_and_elements();
  else
  {
    mesh->remove_orphaned_nodes();
    mesh->update_parallel_id_counts();
  }
}
