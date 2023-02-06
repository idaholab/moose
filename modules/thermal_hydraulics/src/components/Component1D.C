//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Component1D.h"
#include "THMMesh.h"

InputParameters
Component1D::validParams()
{
  InputParameters params = GeneratedMeshComponent::validParams();
  return params;
}

Component1D::Component1D(const InputParameters & parameters) : GeneratedMeshComponent(parameters) {}

void
Component1D::buildMeshNodes()
{
  Point p(0, 0, 0);
  for (unsigned int i = 0; i < _node_locations.size(); i++)
  {
    p(0) = _node_locations[i];
    addNode(p);
  }
}

void
Component1D::buildMesh()
{
  buildMeshNodes();

  MeshBase & the_mesh = mesh().getMesh();
  BoundaryInfo & boundary_info = the_mesh.get_boundary_info();

  // create nodeset for all nodes for this component
  _nodeset_id = mesh().getNextBoundaryId();
  _nodeset_name = name();
  boundary_info.nodeset_name(_nodeset_id) = _nodeset_name;

  // Check that the number of nodes is consistent with the number of nodes in case component
  // developers screw up (typically in buildMeshNodes() call)
  if (usingSecondOrderMesh())
  {
    if (_node_ids.size() != (2 * _n_elem + 1))
      mooseError(name(),
                 ": Inconsistent number of nodes and elements. You have ",
                 _n_elem,
                 " elements and ",
                 _node_ids.size(),
                 " nodes.");
  }
  else
  {
    if (_node_ids.size() != _n_elem + 1)
      mooseError(name(),
                 ": Inconsistent number of nodes and elements. You have ",
                 _n_elem,
                 " elements and ",
                 _node_ids.size(),
                 " nodes.");
  }

  for (auto & node_id : _node_ids)
  {
    const Node * nd = the_mesh.node_ptr(node_id);
    boundary_info.add_node(nd, _nodeset_id);
  }

  // elems
  BoundaryID bc_id_inlet = mesh().getNextBoundaryId();
  BoundaryID bc_id_outlet = mesh().getNextBoundaryId();
  auto & binfo = mesh().getMesh().get_boundary_info();
  for (unsigned int i = 0; i < _n_elem; i++)
  {
    Elem * elem = nullptr;
    if (usingSecondOrderMesh())
      elem = addElementEdge3(_node_ids[2 * i], _node_ids[2 * i + 2], _node_ids[2 * i + 1]);
    else
      elem = addElementEdge2(_node_ids[i], _node_ids[i + 1]);

    // BCs
    if (i == 0)
    {
      Point pt = _position;
      _connections[Component1DConnection::IN].push_back(
          Connection(pt, elem->node_ptr(0), bc_id_inlet, -1));
      boundary_info.add_side(elem, 0, bc_id_inlet);
      binfo.sideset_name(bc_id_inlet) = genName(name(), "in");
    }
    if (i == (_n_elem - 1))
    {
      Point pt = _position + _length * _dir;
      _connections[Component1DConnection::OUT].push_back(
          Connection(pt, elem->node_ptr(1), bc_id_outlet, 1));
      boundary_info.add_side(elem, 1, bc_id_outlet);
      binfo.sideset_name(bc_id_outlet) = genName(name(), "out");
    }
  }

  if (_axial_region_names.size() > 0)
  {
    unsigned int k = 0;
    for (unsigned int i = 0; i < _axial_region_names.size(); i++)
    {
      const std::string & region_name = _axial_region_names[i];
      SubdomainID subdomain_id = mesh().getNextSubdomainId();
      setSubdomainInfo(subdomain_id, genName(name(), region_name));

      for (unsigned int j = 0; j < _n_elems[i]; j++, k++)
      {
        dof_id_type elem_id = _elem_ids[k];
        mesh().elemPtr(elem_id)->subdomain_id() = subdomain_id;
      }
    }
  }
  else
  {
    SubdomainID subdomain_id = mesh().getNextSubdomainId();
    setSubdomainInfo(subdomain_id, name());

    for (auto && id : _elem_ids)
      mesh().elemPtr(id)->subdomain_id() = subdomain_id;
  }

  // Update the mesh
  mesh().update();
}

bool
Component1D::usingSecondOrderMesh() const
{
  return false;
}

unsigned int
Component1D::getNodesetID() const
{
  checkSetupStatus(MESH_PREPARED);

  return _nodeset_id;
}

const BoundaryName &
Component1D::getNodesetName() const
{
  checkSetupStatus(MESH_PREPARED);

  return _nodeset_name;
}

const std::vector<Component1D::Connection> &
Component1D::getConnections(Component1DConnection::EEndType end_type) const
{
  checkSetupStatus(MESH_PREPARED);

  std::map<Component1DConnection::EEndType, std::vector<Connection>>::const_iterator it =
      _connections.find(end_type);
  if (it != _connections.end())
    return it->second;
  else
    mooseError(name(), ": Invalid end type (", end_type, ").");
}
