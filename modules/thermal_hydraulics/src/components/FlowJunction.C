//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowJunction.h"
#include "GeometricalFlowComponent.h"
#include "THMMesh.h"

InputParameters
FlowJunction::validParams()
{
  InputParameters params = FlowConnection::validParams();
  params.addPrivateParam<std::string>("component_type", "flow_junction");
  params.addRequiredParam<std::vector<BoundaryName>>("connections", "Junction connections");
  return params;
}

FlowJunction::FlowJunction(const InputParameters & params)
  : FlowConnection(params),

    _junction_uo_name(genName(name(), "junction_uo"))
{
  const std::vector<BoundaryName> & connections =
      getParam<std::vector<BoundaryName>>("connections");
  for (const auto & connection_string : connections)
    addConnection(connection_string);
}

void
FlowJunction::setupMesh()
{
  FlowConnection::setupMesh();

  const BoundaryID boundary_id = _mesh.getNextBoundaryId();

  auto & boundary_info = _mesh.getMesh().get_boundary_info();

  for (const auto & connection : getConnections())
  {
    const std::string & comp_name = connection._geometrical_component_name;

    if (hasComponentByName<GeometricalFlowComponent>(comp_name))
    {
      const GeometricalFlowComponent & gc = getComponentByName<GeometricalFlowComponent>(comp_name);
      for (auto && conn : gc.getConnections(connection._end_type))
        // add connection's node to nodeset of all nodes connected to this zero-D component
        boundary_info.add_node(conn._node, boundary_id);
    }
  }

  // name the nodeset/sideset corresponding to the nodes of all connected flow channel ends
  _mesh.setBoundaryName(boundary_id, name());
}
