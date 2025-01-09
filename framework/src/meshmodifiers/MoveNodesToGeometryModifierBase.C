//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MoveNodesToGeometryModifierBase.h"
#include "MooseMesh.h"
#include "libmesh/mesh_base.h"

InputParameters
MoveNodesToGeometryModifierBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Snap refined nodes on a given boundary or block to a given geometry.");
  params.addParam<std::vector<BoundaryName>>(
      "boundary", {}, "List of boundaries whose nodes are snapped to a given geometry");
  params.addParam<std::vector<SubdomainName>>(
      "block", {}, "List of blocks whose nodes are snapped to a given geometry");

  // By default don't execute
  params.set<ExecFlagEnum>("execute_on") = "NONE";

  return params;
}

MoveNodesToGeometryModifierBase::MoveNodesToGeometryModifierBase(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _boundary_ids(_mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("boundary"))),
    _subdomain_ids(_mesh.getSubdomainIDs(getParam<std::vector<SubdomainName>>("block")))
{
}

void
MoveNodesToGeometryModifierBase::initialize()
{
}

void
MoveNodesToGeometryModifierBase::execute()
{
  snapNodes();
}

void
MoveNodesToGeometryModifierBase::finalize()
{
}

void
MoveNodesToGeometryModifierBase::meshChanged()
{
  snapNodes();
}

void
MoveNodesToGeometryModifierBase::snapNodes()
{
  auto & mesh = _mesh.getMesh();

  // go over boundaries
  for (auto & boundary_id : _boundary_ids)
  {
    auto node_ids = _mesh.getNodeList(boundary_id);
    for (auto & node_id : node_ids)
    {
      auto & node = mesh.node_ref(node_id);

      snapNode(node);
    }
  }

  // go over blocks
  MeshBase::node_iterator node = mesh.active_nodes_begin();
  MeshBase::node_iterator node_end = mesh.active_nodes_end();
  for (; node != node_end; ++node)
  {
    // check if node is part of any of the selected blocks
    const auto & node_blocks = _mesh.getNodeBlockIds(**node);
    for (const auto subdomain_id : _subdomain_ids)
      if (node_blocks.count(subdomain_id))
      {
        snapNode(**node);
        break;
      }
  }
}
