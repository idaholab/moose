//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshSideSet.h"
#include "MooseMesh.h"

#include "libmesh/mesh_modification.h"

template <>
InputParameters
validParams<MeshSideSet>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addClassDescription("Add lower dimensional elements along the faces contained in a side "
                             "set to set up mixed dimensional problems");
  params.addRequiredParam<std::vector<BoundaryName>>("boundaries",
                                                     "The name of the boundary to mesh");
  params.addRequiredParam<SubdomainID>(
      "block_id", "Subdomain id to set for the new elements along the boundary");
  params.addParam<SubdomainName>(
      "block_name", "Subdomain name to set for the new elements along the boundary (optional)");
  return params;
}

MeshSideSet::MeshSideSet(const InputParameters & parameters)
  : MeshModifier(parameters), _block_id(parameters.get<SubdomainID>("block_id"))
{
}

void
MeshSideSet::modify()
{
  // this modifier is not designed for working with distributed mesh
  _mesh_ptr->errorIfDistributedMesh("BreakBoundaryOnSubdomain");

  // Reference the the libMesh::MeshBase
  auto & mesh = _mesh_ptr->getMesh();
  auto & boundary_info = mesh.get_boundary_info();

  // get IDs of all boundaries to be broken
  std::set<BoundaryID> mesh_boundary_ids;
  if (isParamValid("boundaries"))
  {
    auto & boundary_names = getParam<std::vector<BoundaryName>>("boundaries");
    for (auto & boundary_name : boundary_names)
      mesh_boundary_ids.insert(_mesh_ptr->getBoundaryID(boundary_name));
  }
  else
    mesh_boundary_ids = boundary_info.get_boundary_ids();

  _mesh_ptr->buildBndElemList();
  for (auto it = _mesh_ptr->bndElemsBegin(); it != _mesh_ptr->bndElemsEnd(); ++it)
    if (mesh_boundary_ids.count((*it)->_bnd_id) > 0)
    {
      Elem * elem = (*it)->_elem;
      auto s = (*it)->_side;

      // build element from the side
      std::unique_ptr<Elem> side(elem->build_side_ptr(s, false));
      side->processor_id() = elem->processor_id();

      // Add the side set subdomain
      Elem * new_elem = mesh.add_elem(side.release());
      new_elem->subdomain_id() = _block_id;
    }

  // Assign block name, if provided
  if (isParamValid("block_name"))
    mesh.subdomain_name(_block_id) = getParam<SubdomainName>("block_name");
}
