//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BlockToMeshConverterGenerator.h"
#include "CastUniquePointer.h"
#include "libmesh/elem.h"
#include "MooseMeshUtils.h"

registerMooseObject("MooseApp", BlockToMeshConverterGenerator);

InputParameters
BlockToMeshConverterGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription(
      "Converts a block (subdomain) from a mesh into a stand-alone mesh with one block in it.");

  // list params
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<SubdomainName>>(
      "target_blocks",
      "The (list of) blocks (or 'subdomains') we wish to have moved to a new mesh (by name, not "
      "ID)");

  return params;
}

BlockToMeshConverterGenerator::BlockToMeshConverterGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _target_blocks(getParam<std::vector<SubdomainName>>("target_blocks"))
{
}

std::unique_ptr<MeshBase>
BlockToMeshConverterGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  if (!mesh->is_replicated())
    mooseError("BlockToMeshConverterGenerator is not implemented for distributed meshes.");

  auto new_mesh = buildMeshBaseObject();

  std::vector<subdomain_id_type> target_block_ids =
      MooseMeshUtils::getSubdomainIDs((*mesh), _target_blocks);

  // Check that the block ids/names exist in the mesh
  std::set<SubdomainID> mesh_blocks;
  mesh->subdomain_ids(mesh_blocks);

  for (std::size_t i = 0; i < target_block_ids.size(); ++i)
    if (target_block_ids[i] == Moose::INVALID_BLOCK_ID || !mesh_blocks.count(target_block_ids[i]))
    {
      paramError("target_blocks",
                 "The target_block '",
                 getParam<std::vector<SubdomainName>>("target_blocks")[i],
                 "' was not found within the mesh.");
    }

  // know which nodes have already been inserted, by tracking the old mesh's node's ids'
  std::unordered_map<dof_id_type, dof_id_type> old_new_node_map;

  for (subdomain_id_type target_block_id : target_block_ids)
  {

    for (auto elem : mesh->active_subdomain_elements_ptr_range(target_block_id))
    {
      // make a deep copy so that mutiple meshs' destructors don't segfault at program termination
      auto copy = elem->build(elem->type());

      // index of node in the copy element must be managed manually as there is no intelligent
      // insert method
      dof_id_type copy_n_index = 0;

      // correctly assign new copies of nodes, loop over nodes
      for (dof_id_type i : elem->node_index_range())
      {
        auto & n = *elem->node_ptr(i);

        if (old_new_node_map.count(n.id()))
        {
          // case where we have already inserted this particular point before
          // then we need to find the already-inserted one and hook it up right
          // to it's respective element
          copy->set_node(copy_n_index++) = new_mesh->node_ptr(old_new_node_map[n.id()]);
        }
        else
        {
          // case where we've NEVER inserted this particular point before
          // add them both to the element and the mesh

          // Nodes' IDs are their indexes in the nodes' respective mesh
          // If we set them as invalid they are automatically re-assigned
          // to be the last element in the mesh's node array
          std::unique_ptr<Node> node_copy = Node::build(elem->point(i), Node::invalid_id);

          // Add to mesh. This method call will automatically re-assign the id; so must call this
          // first
          new_mesh->add_node(node_copy.get());

          // Add to element copy (manually)
          copy->set_node(copy_n_index++) = node_copy.get();

          // remember the (old) ID
          old_new_node_map[n.id()] = node_copy.release()->id();
        }
      }

      // it is ok to release the copy element into the mesh because derived meshes class
      // (ReplicatedMesh, DistributedMesh) manage their own elements, will delete them
      new_mesh->add_elem(copy.release());
    }
  }
  new_mesh->prepare_for_use();

  return dynamic_pointer_cast<MeshBase>(new_mesh);
}
