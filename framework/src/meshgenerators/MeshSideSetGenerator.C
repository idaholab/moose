//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshSideSetGenerator.h"
#include "BndElement.h"
#include "CastUniquePointer.h"

#include "libmesh/mesh_modification.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"

#include <typeinfo>

registerMooseObject("MooseApp", MeshSideSetGenerator);

defineLegacyParams(MeshSideSetGenerator);

InputParameters
MeshSideSetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Add lower dimensional elements along the faces contained in a side "
                             "set to set up mixed dimensional problems");
  params.addRequiredParam<std::vector<BoundaryName>>("boundaries",
                                                     "The names of the boundaries to mesh");
  params.addRequiredParam<subdomain_id_type>(
      "block_id", "Subdomain id to set for the new elements along the boundary");
  params.addParam<SubdomainName>(
      "block_name", "Subdomain name to set for the new elements along the boundary (optional)");

  return params;
}

MeshSideSetGenerator::MeshSideSetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _block_id(parameters.get<subdomain_id_type>("block_id"))
{
  if (typeid(_input).name() == typeid(std::unique_ptr<DistributedMesh>).name())
    mooseError("MeshSideSetGenerator only works with ReplicatedMesh");
}

std::unique_ptr<MeshBase>
MeshSideSetGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  auto & boundary_info = mesh->get_boundary_info();

  // get IDs of all boundaries with which a new block is created
  std::set<boundary_id_type> mesh_boundary_ids;
  if (isParamValid("boundaries"))
  {
    auto & boundary_names = getParam<std::vector<BoundaryName>>("boundaries");
    for (auto & boundary_name : boundary_names)
      mesh_boundary_ids.insert(boundary_info.get_id_by_name(boundary_name));
  }
  else
    mesh_boundary_ids = boundary_info.get_boundary_ids();

  // Equivalent to MooseMesh::buildBndElemList()
  auto bc_tuples = boundary_info.build_active_side_list();
  int n = bc_tuples.size();
  std::vector<std::unique_ptr<BndElement>> bnd_elems;
  std::map<boundary_id_type, std::set<dof_id_type>> bnd_elem_ids;
  bnd_elems.reserve(n);
  for (const auto & t : bc_tuples)
  {
    auto elem_id = std::get<0>(t);
    auto side_id = std::get<1>(t);
    auto bc_id = std::get<2>(t);

    std::unique_ptr<BndElement> bndElem =
        libmesh_make_unique<BndElement>(mesh->elem_ptr(elem_id), side_id, bc_id);
    bnd_elems.push_back(std::move(bndElem));
    bnd_elem_ids[bc_id].insert(elem_id);
  }

  for (auto it = bnd_elems.begin(); it != bnd_elems.end(); ++it)
    if (mesh_boundary_ids.count((*it)->_bnd_id) > 0)
    {
      Elem * elem = (*it)->_elem;
      auto s = (*it)->_side;

      // build element from the side
      std::unique_ptr<Elem> side(elem->build_side_ptr(s, false));
      side->processor_id() = elem->processor_id();

      // Add the side set subdomain
      Elem * new_elem = mesh->add_elem(side.release());
      new_elem->subdomain_id() = _block_id;
    }

  // Assign block name, if provided
  if (isParamValid("block_name"))
    mesh->subdomain_name(_block_id) = getParam<SubdomainName>("block_name");

  return dynamic_pointer_cast<MeshBase>(mesh);
}
