//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeometryBase.h"
#include "MooseMesh.h"
#include "libmesh/mesh_base.h"

InputParameters
GeometryBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Snap refined nodes on a given boundary to a given geometry");
  params.addParam<std::vector<BoundaryName>>(
      "boundary", "List of boundaries whose nodes are snapped to a given geometry");
  return params;
}

GeometryBase::GeometryBase(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _boundary_ids(_mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("boundary")))
{
}

void
GeometryBase::initialize()
{
}

void
GeometryBase::execute()
{
}

void
GeometryBase::finalize()
{
}

void
GeometryBase::meshChanged()
{
  auto & mesh = _mesh.getMesh();

  for (auto & boundary_id : _boundary_ids)
  {
    auto node_ids = _mesh.getNodeList(boundary_id);
    for (auto & node_id : node_ids)
    {
      auto & node = mesh.node_ref(node_id);

      snapNode(node);
    }
  }
}
