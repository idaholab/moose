//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshComponentJunction.h"
#include "FileMeshComponent.h"
#include "THMMesh.h"

InputParameters
FileMeshComponentJunction::validParams()
{
  InputParameters params = FileMeshComponentConnection::validParams();
  params.addPrivateParam<std::string>("component_type", "junction");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "connections",
      "Junction connections. This must include the suffix :in / :out to declare the intent");
  return params;
}

FileMeshComponentJunction::FileMeshComponentJunction(const InputParameters & params)
  : FileMeshComponentConnection(params)
{
  const std::vector<BoundaryName> & connections =
      getParam<std::vector<BoundaryName>>("connections");
  for (const auto & connection_string : connections)
    addConnection(connection_string);
}

void
FileMeshComponentJunction::setupMesh()
{
  FileMeshComponentConnection::setupMesh();

  mesh().buildNodeList();
  const BoundaryID boundary_id = mesh().getNextBoundaryId();

  auto & boundary_info = mesh().getMesh().get_boundary_info();

  for (const auto & connection : getConnections())
  {
    const std::string & comp_name = connection._component_name;

    if (hasComponentByName<FileMeshComponent>(comp_name))
    {
      // const FileMeshComponent & comp = getComponentByName<FileMeshComponent>(comp_name);
      // for (auto && conn : comp.getConnections(connection._end_type))
      for (const auto node_id :
           mesh().nodeSetNodes().at(mesh().getBoundaryID(connection._boundary_name)))
        boundary_info.add_node(node_id, boundary_id);
    }
  }

  // name the nodeset/sideset corresponding to the nodes of all connected component ends
  boundary_info.nodeset_name(boundary_id) = name();
  boundary_info.sideset_name(boundary_id) = name();

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
FileMeshComponentJunction::initSecondary()
{
  FileMeshComponentConnection::initSecondary();

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
FileMeshComponentJunction::check() const
{
  FileMeshComponentConnection::check();

  if (_connections.size() == 0)
    logError("There must be at least one connection.");
}
