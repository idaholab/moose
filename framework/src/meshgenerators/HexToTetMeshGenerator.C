//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HexToTetMeshGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/cell_hex8.h"

registerMooseObject("MooseApp", HexToTetMeshGenerator);

InputParameters
HexToTetMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh to convert");
  params.addClassDescription("Converts from a hexahedral (HEX8) mesh to a tetrahedral (TET4) mesh.");
  return params;
}

HexToTetMeshGenerator::HexToTetMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input"))
{
  _tet4_nodes.push_back({4, 1, 0, 7});
  _tet4_nodes.push_back({3, 0, 1, 7});
  _tet4_nodes.push_back({4, 5, 1, 7});
  _tet4_nodes.push_back({6, 1, 5, 7});
  _tet4_nodes.push_back({2, 3, 1, 7});
  _tet4_nodes.push_back({6, 2, 1, 7});

//    {0, 2, 1}, 
//    {0, 1, 3}, 
//    {1, 2, 3}, 
//    {2, 0, 3}  

  // there are six faces on the parent element, which we need to map to the faces of
  // the daughter elements in order to properly retain the sideset information
//  _daughter_face_to_parent_face.resize(TET4_ELEM_PER_HEX8);
//  _daughter_face_to_parent_face[0].push_back({0, 0});
//  _daughter_face_to_parent_face[0].push_back({1, 0});
}

std::unique_ptr<MeshBase>
HexToTetMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  auto & boundary_info = mesh->get_boundary_info();
  const auto & original_boundaries = boundary_info.get_boundary_ids();

  // TODO: no real reason for this restriction, just didn't need it in the first pass
  if (!mesh->is_replicated())
    mooseError("This mesh generator does not yet support distributed mesh implementations!");

  // check that each input element is of type HEX8
  for (const auto & elem : mesh->element_ptr_range())
    if (elem->type() != HEX8)
      mooseError("This mesh generator can only be applied to meshes with HEX8 elements!");

  // store all information from the incoming mesh that is needed to rebuild it from scratch.
  std::vector<unsigned int> elem_block_ids;
  std::vector<std::vector<Point>> elem_nodes;
  std::vector<std::vector<unsigned int>> elem_node_ids;
  elem_nodes.resize(mesh->n_elem());
  elem_node_ids.resize(mesh->n_elem());

  // index by element, then face, then for boundary IDs
  std::vector<std::vector<std::vector<boundary_id_type>>> elem_face_boundary_ids;
  elem_face_boundary_ids.resize(mesh->n_elem());
  for (auto & e : elem_face_boundary_ids)
    e.resize(Hex8::num_sides);

  // get the coordinates of all the original nodes, indexed by element. We add all the "actual"
  // nodes, and then also the element centroid (which we will use as the additional node tying
  // all the tet4 elements together)
  int i = 0;
  auto max_node_id = mesh->max_node_id();
  for (const auto & elem : mesh->element_ptr_range())
  {
    elem_block_ids.push_back(elem->subdomain_id());

    for (unsigned int j = 0; j < Hex8::num_nodes; ++j)
    {
      elem_nodes[i].push_back(elem->node_ref(j));
      elem_node_ids[i].push_back(elem->node_ref(j).id());
    }

//    std::vector<boundary_id_type> b;
//    for (unsigned short int s = 0; s < Hex8::num_sides; ++s)
//    {
//      boundary_info.boundary_ids(elem, s, b);
//      //elem_face_boundary_ids[i][s].push_back(b);
//    }

    i++;
  }

  mesh->clear();

  // create the nodes
  for (unsigned int i = 0; i < elem_nodes.size(); ++i)
  {
    for (unsigned int j = 0; j < elem_nodes[i].size(); ++j)
      mesh->add_point(elem_nodes[i][j], elem_node_ids[i][j]);
  }

  // create the elements
  int id = 0;
  for (unsigned int i = 0; i < elem_nodes.size(); ++i)
  {
    for (unsigned int j = 0; j < TET4_ELEM_PER_HEX8; ++j)
    {
      Elem * elem = new Tet4;
      elem->set_id(id++);
      elem->subdomain_id() = elem_block_ids[i];
      mesh->add_elem(elem);

      auto nodes = _tet4_nodes[j];
      int n_idx = 0;
      for (const auto & n : nodes)
      {
        auto node_ptr = mesh->node_ptr(elem_node_ids[i][n]);
        elem->set_node(n_idx++) = node_ptr;
      }
    }
  }

  mesh->prepare_for_use();

  return dynamic_pointer_cast<MeshBase>(mesh);
}
