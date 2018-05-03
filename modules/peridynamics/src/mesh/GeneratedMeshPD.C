//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneratedMeshPD.h"

#include "libmesh/unstructured_mesh.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/utility.h"

registerMooseObject("PeridynamicsApp", GeneratedMeshPD);

template <>
InputParameters
validParams<GeneratedMeshPD>()
{
  InputParameters params = validParams<MeshBasePD>();
  params.addClassDescription("Class for generating peridynamic mesh using regular grids");

  params.addRangeCheckedParam<unsigned int>(
      "nx", 1, "nx > 0", "Number of elements in the X direction");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");

  return params;
}

GeneratedMeshPD::GeneratedMeshPD(const InputParameters & parameters)
  : MeshBasePD(parameters),
    _nx(getParam<unsigned int>("nx")),
    _xmin(getParam<Real>("xmin")),
    _ymin(getParam<Real>("ymin")),
    _zmin(getParam<Real>("zmin")),
    _xmax(getParam<Real>("xmax")),
    _ymax(getParam<Real>("ymax")),
    _zmax(getParam<Real>("zmax"))
{
  _dim = getParam<MooseEnum>("dim");
}

std::unique_ptr<MooseMesh>
GeneratedMeshPD::safeClone() const
{
  return libmesh_make_unique<GeneratedMeshPD>(*this);
}

void
GeneratedMeshPD::init()
{
  if (_dim == 2)
    init2DRectangular(); // 2D rectangular domain
  else
    init3DRectangular(); // 3D rectangular domain

  MooseMesh::init();
}

void
GeneratedMeshPD::buildMesh()
{
  UnstructuredMesh & mesh = dynamic_cast<UnstructuredMesh &>(getMesh());
  mesh.clear();
  mesh.set_mesh_dimension(_dim);
  mesh.set_spatial_dimension(_dim);
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  if (_dim == 2)
    build2DRectangular(mesh, boundary_info); // 2D rectangular domain
  else
    build3DRectangular(mesh, boundary_info); // 3D rectangular domain

  _console << "Mesh Information:" << '\n';
  _console << "  Number of Nodes:         " << _total_nodes << '\n';
  _console << "  Number of Bonds:         " << _total_bonds << '\n';
  _console << '\n';

  // prepare for use
  mesh.prepare_for_use(/*skip_renumber =*/true);
}

void
GeneratedMeshPD::init2DRectangular()
{
  _spacing = (_xmax - _xmin) / _nx;
  Real horizon = MeshBasePD::computeHorizon(_spacing);
  unsigned int ny = static_cast<int>((_ymax - _ymin) / _spacing);

  _total_nodes = 0;
  Real X, Y;
  for (unsigned int j = 0; j < ny; ++j)
    for (unsigned int i = 0; i < _nx; ++i)
    {
      X = _xmin + i * _spacing + 0.5 * _spacing;
      Y = _ymin + j * _spacing + 0.5 * _spacing;
      bool inside = false;
      for (unsigned int n = 0; n < _cracks_start.size(); ++n)
      {
        inside =
            inside || MeshBasePD::checkInside(
                          _cracks_start[n], _cracks_end[n], Point(X, Y, 0.0), _cracks_width[n]);
      }
      if (!inside)
        ++_total_nodes;
    }

  _pdnode.resize(_total_nodes);
  _node_neighbors.resize(_total_nodes);
  _node_n_nearest_neighbors.resize(_total_nodes);
  _node_bonds.resize(_total_nodes);
  _dg_nodeinfo.resize(_total_nodes);
  _dg_bond_volumesum.resize(_total_nodes);
  _dg_node_volumesum.resize(_total_nodes);

  // define nodal coordinates
  unsigned int k = 0;
  for (unsigned int j = 0; j < ny; ++j)
    for (unsigned int i = 0; i < _nx; ++i)
    {
      X = _xmin + i * _spacing + 0.5 * _spacing;
      Y = _ymin + j * _spacing + 0.5 * _spacing;
      bool inside = false;
      for (unsigned int n = 0; n < _cracks_start.size(); ++n)
      {
        inside =
            inside || MeshBasePD::checkInside(
                          _cracks_start[n], _cracks_end[n], Point(X, Y, 0.0), _cracks_width[n]);
      }
      if (!inside)
      {
        _pdnode[k].coord = Point(X, Y, 0.0);
        _pdnode[k].mesh_spacing = _spacing;
        _pdnode[k].horizon = horizon;
        _pdnode[k].volume = _spacing * _spacing;
        _pdnode[k].volumesum = 0.0;
        _pdnode[k].blockID = 0;
        ++k;
      }
    }

  // search node neighbor
  MeshBasePD::findNodeNeighbor();

  // setup node info for deformation gradient
  MeshBasePD::setupDGNodeInfo();

  // find bonds for each node
  _total_bonds = 0;
  for (unsigned int i = 0; i < _total_nodes; ++i)
    _total_bonds += _node_neighbors[i].size();
  _total_bonds /= 2;

  k = 0;
  for (unsigned int i = 0; i < _total_nodes; ++i)
    for (unsigned int j = 0; j < _node_neighbors[i].size(); ++j)
      if (_node_neighbors[i][j] > i)
      {
        // build the bond list for each node
        _node_bonds[i].push_back(k);
        _node_bonds[_node_neighbors[i][j]].push_back(k);
        ++k;
      }
}

void
GeneratedMeshPD::init3DRectangular()
{
  _spacing = (_xmax - _xmin) / _nx;
  Real horizon = MeshBasePD::computeHorizon(_spacing);
  unsigned int ny = static_cast<int>((_ymax - _ymin) / _spacing);
  unsigned int nz = static_cast<int>((_zmax - _zmin) / _spacing);

  _total_nodes = 0;
  Real X, Y, Z;
  for (unsigned int n = 0; n < nz; ++n)
    for (unsigned int j = 0; j < ny; ++j)
      for (unsigned int i = 0; i < _nx; ++i)
      {
        X = _xmin + i * _spacing + 0.5 * _spacing;
        Y = _ymin + j * _spacing + 0.5 * _spacing;
        Z = _zmin + n * _spacing + 0.5 * _spacing;
        bool inside = false;
        for (unsigned int n = 0; n < _cracks_start.size(); ++n)
        {
          inside =
              inside || MeshBasePD::checkInside(
                            _cracks_start[n], _cracks_end[n], Point(X, Y, Z), _cracks_width[n]);
        }
        if (!inside)
          ++_total_nodes;
      }

  _pdnode.resize(_total_nodes);
  _node_neighbors.resize(_total_nodes);
  _node_n_nearest_neighbors.resize(_total_nodes);
  _node_bonds.resize(_total_nodes);
  _dg_nodeinfo.resize(_total_nodes);
  _dg_bond_volumesum.resize(_total_nodes);
  _dg_node_volumesum.resize(_total_nodes);

  // define nodal coordinates
  unsigned int k = 0;
  for (unsigned int n = 0; n < nz; ++n)
    for (unsigned int j = 0; j < ny; ++j)
      for (unsigned int i = 0; i < _nx; ++i)
      {
        X = _xmin + i * _spacing + 0.5 * _spacing;
        Y = _ymin + j * _spacing + 0.5 * _spacing;
        Z = _zmin + n * _spacing + 0.5 * _spacing;
        bool inside = false;
        for (unsigned int n = 0; n < _cracks_start.size(); ++n)
        {
          inside =
              inside || MeshBasePD::checkInside(
                            _cracks_start[n], _cracks_end[n], Point(X, Y, Z), _cracks_width[n]);
        }
        if (!inside)
        {
          _pdnode[k].coord = Point(X, Y, Z);
          _pdnode[k].mesh_spacing = _spacing;
          _pdnode[k].horizon = horizon;
          _pdnode[k].volume = _spacing * _spacing * _spacing;
          _pdnode[k].volumesum = 0.0;
          _pdnode[k].blockID = 0;
          ++k;
        }
      }

  // search node neighbor
  MeshBasePD::findNodeNeighbor();

  // setup node info for deformation gradient
  MeshBasePD::setupDGNodeInfo();

  // find bonds for each node
  _total_bonds = 0;
  for (unsigned int i = 0; i < _total_nodes; ++i)
    _total_bonds += _node_neighbors[i].size();
  _total_bonds /= 2;

  k = 0;
  for (unsigned int i = 0; i < _total_nodes; ++i)
    for (unsigned int j = 0; j < _node_neighbors[i].size(); ++j)
      if (_node_neighbors[i][j] > i)
      {
        // build the bond list for each node
        _node_bonds[i].push_back(k);
        _node_bonds[_node_neighbors[i][j]].push_back(k);
        ++k;
      }
}

void
GeneratedMeshPD::build2DRectangular(UnstructuredMesh & mesh, BoundaryInfo & boundary_info)
{
  mesh.reserve_nodes(_total_nodes);
  mesh.reserve_elem(_total_bonds);

  // define mesh nodal coordinates
  for (unsigned int i = 0; i < _total_nodes; ++i)
    mesh.add_point(_pdnode[i].coord, i);

  unsigned int k = 0;
  for (unsigned int i = 0; i < _total_nodes; ++i)
    for (unsigned int j = 0; j < _node_neighbors[i].size(); ++j)
      if (_node_neighbors[i][j] > i)
      {
        Elem * elem = new Edge2;
        elem->set_id(k);
        elem = mesh.add_elem(elem);
        elem->set_node(0) = mesh.node_ptr(i);
        elem->set_node(1) = mesh.node_ptr(_node_neighbors[i][j]);
        elem->subdomain_id() = _pdnode[i].blockID;
        ++k;
      }

  // define boundary nodeset
  for (unsigned int i = 0; i < _total_nodes; ++i)
  {
    Real X = (_pdnode[i].coord)(0);
    Real Y = (_pdnode[i].coord)(1);
    if (X < _xmin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 0);
    if (X > _xmax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 1);
    if (Y < _ymin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 2);
    if (Y > _ymax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 3);
    if (std::sqrt((X - 0.5 * (_xmax + _xmin)) * (X - 0.5 * (_xmax + _xmin)) +
                  (Y - 0.5 * (_ymax + _ymin)) * (Y - 0.5 * (_ymax + _ymin))) < _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 4);
    if (std::abs(Y - 0.5 * (_ymax + _ymin)) < _spacing && X < _xmin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 5);
    if (std::abs(Y - 0.5 * (_ymax + _ymin)) < _spacing && X > _xmax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 6);
    if (std::abs(X - 0.5 * (_xmax + _xmin)) < _spacing && Y < _ymin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 7);
    if (std::abs(X - 0.5 * (_xmax + _xmin)) < _spacing && Y > _ymax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 8);
    if (std::abs(Y - 0.5 * (_ymax + _ymin)) < _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 9);
    if (std::abs(X - 0.5 * (_xmax + _xmin)) < _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 10);
    if (std::abs((_xmax - _xmin) * (_ymin - Y) - (_xmin - X) * (_ymax - _ymin)) /
            std::sqrt(Utility::pow<2>(_xmax - _xmin) + Utility::pow<2>(_ymax - _ymin)) <
        0.1 * _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 11);
    if (std::abs((_xmax - _xmin) * (_ymax - Y) - (_xmax - X) * (_ymax - _ymin)) /
            std::sqrt(Utility::pow<2>(_xmax - _xmin) + Utility::pow<2>(_ymax - _ymin)) <
        0.1 * _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 12);
    if (X < _xmin + _spacing && Y < _ymin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 13);
    if (X > _xmax - _spacing && Y < _ymin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 14);
    if (X > _xmax - _spacing && Y > _ymax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 15);
    if (X < _xmin + _spacing && Y > _ymax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 16);
    boundary_info.add_node(mesh.node_ptr(i), 999);
  }
  boundary_info.nodeset_name(0) = "Left";
  boundary_info.nodeset_name(1) = "Right";
  boundary_info.nodeset_name(2) = "Bottom";
  boundary_info.nodeset_name(3) = "Top";
  boundary_info.nodeset_name(4) = "Center";
  boundary_info.nodeset_name(5) = "LeftCenter";
  boundary_info.nodeset_name(6) = "RightCenter";
  boundary_info.nodeset_name(7) = "BottomCenter";
  boundary_info.nodeset_name(8) = "TopCenter";
  boundary_info.nodeset_name(9) = "XCenterLine";
  boundary_info.nodeset_name(10) = "YCenterLine";
  boundary_info.nodeset_name(11) = "UpDiag";
  boundary_info.nodeset_name(12) = "DownDiag";
  boundary_info.nodeset_name(13) = "LeftBottomCorner";
  boundary_info.nodeset_name(14) = "RightBottomCorner";
  boundary_info.nodeset_name(15) = "RightTopCorner";
  boundary_info.nodeset_name(16) = "LeftTopCorner";
  boundary_info.nodeset_name(999) = "All";
}

void
GeneratedMeshPD::build3DRectangular(UnstructuredMesh & mesh, BoundaryInfo & boundary_info)
{
  mesh.reserve_nodes(_total_nodes);
  mesh.reserve_elem(_total_bonds);

  // define mesh nodal coordinates
  for (unsigned int i = 0; i < _total_nodes; ++i)
    mesh.add_point(_pdnode[i].coord, i);

  unsigned int k = 0;
  for (unsigned int i = 0; i < _total_nodes; ++i)
    for (unsigned int j = 0; j < _node_neighbors[i].size(); ++j)
      if (_node_neighbors[i][j] > i)
      {
        Elem * elem = new Edge2;
        elem->set_id(k);
        elem = mesh.add_elem(elem);
        elem->set_node(0) = mesh.node_ptr(i);
        elem->set_node(1) = mesh.node_ptr(_node_neighbors[i][j]);
        elem->subdomain_id() = _pdnode[i].blockID;
        ++k;
      }

  // define boundary nodeset
  for (unsigned int i = 0; i < _total_nodes; ++i)
  {
    Real X = (_pdnode[i].coord)(0);
    Real Y = (_pdnode[i].coord)(1);
    Real Z = (_pdnode[i].coord)(2);
    if (X < _xmin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 0);
    if (X > _xmax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 1);
    if (Y < _ymin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 2);
    if (Y > _ymax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 3);
    if (Z < _zmin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 4);
    if (Z > _zmax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 5);
    if (std::sqrt((X - 0.5 * (_xmax - _xmin)) * (X - 0.5 * (_xmax - _xmin)) +
                  (Y - 0.5 * (_ymax - _ymin)) * (Y - 0.5 * (_ymax - _ymin)) +
                  (Z - 0.5 * (_zmax - _zmin)) * (Z - 0.5 * (_zmax - _zmin))) < _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 6);
    if (std::abs(Y - 0.5 * (_ymax - _ymin)) < _spacing &&
        std::abs(Z - 0.5 * (_zmax - _zmin)) < _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 7);
    if (std::abs(X - 0.5 * (_xmax - _xmin)) < _spacing &&
        std::abs(Z - 0.5 * (_zmax - _zmin)) < _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 8);
    if (std::abs(X - 0.5 * (_xmax - _xmin)) < _spacing &&
        std::abs(Z - 0.5 * (_ymax - _ymin)) < _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 9);
    if (std::abs(X - 0.5 * (_zmax - _zmin)) < _spacing && X < _xmin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 10);
    if (std::abs(X - 0.5 * (_ymax - _ymin)) < _spacing && X < _xmin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 11);
    if (std::abs(X - 0.5 * (_zmax - _zmin)) < _spacing && X > _xmax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 12);
    if (std::abs(X - 0.5 * (_ymax - _ymin)) < _spacing && X > _xmax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 13);
    if (std::abs(X - 0.5 * (_zmax - _zmin)) < _spacing && Y < _ymin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 14);
    if (std::abs(X - 0.5 * (_xmax - _xmin)) < _spacing && Y < _ymin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 15);
    if (std::abs(X - 0.5 * (_zmax - _zmin)) < _spacing && Y > _ymax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 16);
    if (std::abs(X - 0.5 * (_xmax - _xmin)) < _spacing && Y > _ymax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 17);
    if (std::abs(X - 0.5 * (_ymax - _ymin)) < _spacing && Z < _zmin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 18);
    if (std::abs(X - 0.5 * (_xmax - _xmin)) < _spacing && Z < _zmin + _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 19);
    if (std::abs(X - 0.5 * (_ymax - _ymin)) < _spacing && Z > _zmax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 20);
    if (std::abs(X - 0.5 * (_xmax - _xmin)) < _spacing && Z > _zmax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 21);
    if (Y > _ymax - _spacing && Z > _zmax - _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 22);
    if (std::sqrt(((Utility::pow<2>(_xmin - X) + Utility::pow<2>(_ymin - Y) +
                    Utility::pow<2>(_zmin - Z)) *
                       (Utility::pow<2>(_xmax - _xmin) + Utility::pow<2>(_ymax - _ymin) +
                        Utility::pow<2>(_zmax - _zmin)) -
                   Utility::pow<2>((_xmin - X) * (_xmax - _xmin) + (_ymin - Y) * (_ymax - _ymin) +
                                   (_zmin - Z) * (_zmax - _zmin))) /
                  (Utility::pow<2>(_xmax - _xmin) + Utility::pow<2>(_ymax - _ymin) +
                   Utility::pow<2>(_zmax - _zmin))) < 0.1 * _spacing)
      boundary_info.add_node(mesh.node_ptr(i), 23);
    boundary_info.add_node(mesh.node_ptr(i), 999);
  }
  boundary_info.nodeset_name(0) = "Back";
  boundary_info.nodeset_name(1) = "Front";
  boundary_info.nodeset_name(2) = "Left";
  boundary_info.nodeset_name(3) = "Right";
  boundary_info.nodeset_name(4) = "Bottom";
  boundary_info.nodeset_name(5) = "Top";
  boundary_info.nodeset_name(6) = "Center";
  boundary_info.nodeset_name(7) = "XCenterLine";
  boundary_info.nodeset_name(8) = "YCenterLine";
  boundary_info.nodeset_name(9) = "ZCenterLine";
  boundary_info.nodeset_name(10) = "BackYCenterLine";
  boundary_info.nodeset_name(11) = "BackZCenterLine";
  boundary_info.nodeset_name(12) = "FrontYCenterLine";
  boundary_info.nodeset_name(13) = "FrontZCenterLine";
  boundary_info.nodeset_name(14) = "LeftXCenterLine";
  boundary_info.nodeset_name(15) = "LeftZCenterLine";
  boundary_info.nodeset_name(16) = "RightXCenterLine";
  boundary_info.nodeset_name(17) = "RightZCenterLine";
  boundary_info.nodeset_name(18) = "BottomXCenterLine";
  boundary_info.nodeset_name(19) = "BottomYCenterLine";
  boundary_info.nodeset_name(20) = "TopXCenterLine";
  boundary_info.nodeset_name(21) = "TopYCenterLine";
  boundary_info.nodeset_name(22) = "RightTopLine";
  boundary_info.nodeset_name(23) = "UpDiag";
  boundary_info.nodeset_name(999) = "All";
}
