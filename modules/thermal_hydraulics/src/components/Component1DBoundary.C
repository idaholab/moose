//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Component1DBoundary.h"
#include "THMMesh.h"

InputParameters
Component1DBoundary::validParams()
{
  InputParameters params = Component1DConnection::validParams();
  params.addRequiredParam<BoundaryName>("input", "Name of the input");
  return params;
}

Component1DBoundary::Component1DBoundary(const InputParameters & params)
  : Component1DConnection(params), _input(getParam<BoundaryName>("input"))
{
  addConnection(_input);
  if (getConnections().size() > 0)
  {
    const Connection & connection = getConnections()[0];
    _connected_component_name = connection._component_name;
    _connected_component_end_type = connection._end_type;
  }
}

void
Component1DBoundary::setupMesh()
{
  Component1DConnection::setupMesh();

  if (_nodes.size() > 0)
  {
    _node = _nodes[0];
    _normal = _normals[0];

    // create a nodeset/sideset corresponding to the node of the connected component end
    const BoundaryID boundary_id = mesh().getNextBoundaryId();
    auto & binfo = mesh().getMesh().get_boundary_info();
    binfo.add_node(_node, boundary_id);
    binfo.nodeset_name(boundary_id) = name();
  }
}

void
Component1DBoundary::check() const
{
  Component1DConnection::check();

  checkNumberOfConnections(1);
}
