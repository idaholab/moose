//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodeScrambler.h"
#include "MooseMesh.h"
#include <random>
#include <algorithm>

registerMooseObject("MooseApp", NodeScrambler);

template <>
InputParameters
validParams<NodeScrambler>()
{
  InputParameters params = validParams<MeshModifier>();

  params.addParam<unsigned int>("seed", 1, "Seed for generating random node ID");
  return params;
}

NodeScrambler::NodeScrambler(const InputParameters & parameters)
  : MeshModifier(parameters),
   _seed(getParam<unsigned int>("seed"))
{
}

void
NodeScrambler::modify()
{
  libmesh_assert(this->comm().verify(this->name()));

  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling ElementDeleterBase::modify()");

   MeshBase & mesh = _mesh_ptr->getMesh();
   auto nnodes = mesh.n_nodes();

   std::vector<dof_id_type> node_ids(nnodes);

   std::iota(node_ids.begin(), node_ids.end(), 0);
   std::mt19937 rgen;

   // Seed the engine with an unsigned int
   rgen.seed(_seed);

   std::shuffle(node_ids.begin(), node_ids.end(), rgen);

   std::vector<dof_id_type> old_node_ids;

  for (const auto & node : mesh.node_ptr_range())
    old_node_ids.push_back(node->id());

  for (const auto old_node_id : old_node_ids)
    mesh.renumber_node(old_node_id, nnodes+node_ids[old_node_id]);


  old_node_ids.clear();

  for (const auto & node : mesh.node_ptr_range())
    old_node_ids.push_back(node->id());

  for (const auto old_node_id : old_node_ids)
    mesh.renumber_node(old_node_id, old_node_id-nnodes);

  _mesh_ptr->prepare();
}
