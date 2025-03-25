//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMTriPinMeshGenerator.h"
#include "TriSubChannelMesh.h"
#include "libmesh/edge_edge2.h"
#include <numeric>

registerMooseObject("SubChannelApp", SCMTriPinMeshGenerator);
registerMooseObjectRenamed("SubChannelApp",
                           TriPinMeshGenerator,
                           "06/30/2025 24:00",
                           SCMTriPinMeshGenerator);

InputParameters
SCMTriPinMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The corresponding subchannel mesh");
  params.addClassDescription("Creates a mesh of 1D fuel pins in a triangular lattice arrangement");
  params.addParam<Real>("unheated_length_entry", 0.0, "Unheated length at entry [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addParam<Real>("unheated_length_exit", 0.0, "Unheated length at exit [m]");
  params.addRequiredParam<Real>("pitch", "Pitch [m]");
  params.addRequiredParam<unsigned int>("nrings", "Number of fuel Pin rings per assembly [-]");
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addParam<unsigned int>("block_id", 1, "Domain Index");
  return params;
}

SCMTriPinMeshGenerator::SCMTriPinMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _unheated_length_entry(getParam<Real>("unheated_length_entry")),
    _heated_length(getParam<Real>("heated_length")),
    _unheated_length_exit(getParam<Real>("unheated_length_exit")),
    _pitch(getParam<Real>("pitch")),
    _n_rings(getParam<unsigned int>("nrings")),
    _n_cells(getParam<unsigned int>("n_cells")),
    _block_id(getParam<unsigned int>("block_id"))
{
  Real dz = (_unheated_length_entry + _heated_length + _unheated_length_exit) / _n_cells;
  for (unsigned int i = 0; i < _n_cells + 1; i++)
    _z_grid.push_back(dz * i);
}

std::unique_ptr<MeshBase>
SCMTriPinMeshGenerator::generate()
{

  // Setting up base elements in the mesh
  std::unique_ptr<MeshBase> mesh_base = std::move(_input);
  if (!mesh_base)
    mesh_base = buildMeshBaseObject();

  mesh_base->set_mesh_dimension(3);

  // Defining the Pin positions
  TriSubChannelMesh::rodPositions(_pin_position, _n_rings, _pitch, Point(0, 0));
  auto _nrods = _pin_position.size();

  // Reserving memory in the mesh
  mesh_base->reserve_elem(_n_cells * _nrods);
  mesh_base->reserve_nodes((_n_cells + 1) * _nrods);
  _pin_nodes.resize(_nrods);

  // Defining the extent of the subchanel mesh to append pins mesh
  // to the current subchannel mesh
  unsigned int chancount = 0;
  for (unsigned int j = 0; j < _n_rings - 1; j++)
    chancount += j * 6;
  unsigned int _n_channels = chancount + _nrods - 1 + (_n_rings - 1) * 6 + 6;
  unsigned int node_sub = (_n_cells + 1) * _n_channels;
  unsigned int elem_sub = _n_cells * _n_channels;

  // Add the points in the shape of a rectilinear grid.  The grid is regular
  // on the xy-plane with a spacing of `pitch` between points.  The grid along
  // z is also regular.  Store pointers in the _nodes
  // array so we can keep track of which points are in which pins.
  unsigned int node_id = node_sub;
  for (unsigned int i = 0; i < _nrods; i++)
  {
    _pin_nodes[i].reserve(_n_cells);
    for (unsigned int iz = 0; iz < _n_cells + 1; iz++)
    {
      _pin_nodes[i].push_back(mesh_base->add_point(
          Point(_pin_position[i](0), _pin_position[i](1), _z_grid[iz]), node_id++));
    }
  }

  // Add the elements which in this case are 2-node edges that link each
  // pin nodes vertically.
  unsigned int elem_id = elem_sub;
  for (unsigned int i = 0; i < _nrods; i++)
  {
    for (unsigned int iz = 0; iz < _n_cells; iz++)
    {
      Elem * elem = new Edge2;
      elem->subdomain_id() = _block_id;
      elem->set_id(elem_id++);
      elem = mesh_base->add_elem(elem);
      const int indx1 = (_n_cells + 1) * i + iz + node_sub;
      const int indx2 = (_n_cells + 1) * i + (iz + 1) + node_sub;
      elem->set_node(0) = mesh_base->node_ptr(indx1);
      elem->set_node(1) = mesh_base->node_ptr(indx2);
    }
  }
  mesh_base->subdomain_name(_block_id) = name();
  mesh_base->prepare_for_use();

  // move the meta data into TriSubChannelMesh
  auto & sch_mesh = static_cast<TriSubChannelMesh &>(*_mesh);
  sch_mesh._pin_nodes = _pin_nodes;
  sch_mesh._pin_mesh_exist = true;

  return mesh_base;
}
