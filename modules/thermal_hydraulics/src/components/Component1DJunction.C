//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Component1DJunction.h"
#include "Component1D.h"
#include "THMMesh.h"

InputParameters
Component1DJunction::validParams()
{
  InputParameters params = Component1DConnection::validParams();
  params.addPrivateParam<std::string>("component_type", "junction");
  params.addRequiredParam<std::vector<BoundaryName>>("connections", "Junction connections");
  return params;
}

Component1DJunction::Component1DJunction(const InputParameters & params)
  : Component1DConnection(params)
{
  const std::vector<BoundaryName> & connections =
      getParam<std::vector<BoundaryName>>("connections");
  for (const auto & connection_string : connections)
    addConnection(connection_string);
}

void
Component1DJunction::setupMesh()
{
  Component1DConnection::setupMesh();

  const BoundaryID boundary_id = mesh().getNextBoundaryId();

  auto & boundary_info = mesh().getMesh().get_boundary_info();

  for (const auto & connection : getConnections())
  {
    const std::string & comp_name = connection._component_name;

    if (hasComponentByName<Component1D>(comp_name))
    {
      const Component1D & comp = getComponentByName<Component1D>(comp_name);
      for (auto && conn : comp.getConnections(connection._end_type))
        // add connection's node to nodeset of all nodes connected to this zero-D component
        boundary_info.add_node(conn._node, boundary_id);
    }
  }

  // name the nodeset/sideset corresponding to the nodes of all connected component ends
  boundary_info.nodeset_name(boundary_id) = name();

  const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem = mesh().nodeToElemMap();
  for (auto & nid : _nodes)
  {
    const auto & it = node_to_elem.find(nid);
    if (it == node_to_elem.end())
      mooseError(name(), ": failed to find node ", nid, "in the mesh!");

    const std::vector<dof_id_type> & elems = it->second;
    for (const auto & e : elems)
      _connected_elems.push_back(e);
  }
}

void
Component1DJunction::initSecondary()
{
  Component1DConnection::initSecondary();

  for (auto & eid : _connected_elems)
  {
    const Elem * elem = constMesh().queryElemPtr(eid);
    if (elem != nullptr && elem->processor_id() == processor_id())
      _proc_ids.push_back(elem->processor_id());
    else
      _proc_ids.push_back(0);
  }
  comm().sum(_proc_ids);
}

void
Component1DJunction::check() const
{
  Component1DConnection::check();

  if (_connections.size() == 0)
    logError("There must be at least one connection.");
}
