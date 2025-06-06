//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMTriDuctMeshGenerator.h"
#include "TriSubChannelMesh.h"
#include <cmath>
#include "libmesh/face_quad4.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", SCMTriDuctMeshGenerator);
registerMooseObjectRenamed("SubChannelApp",
                           TriDuctMeshGenerator,
                           "06/30/2025 24:00",
                           SCMTriDuctMeshGenerator);

InputParameters
SCMTriDuctMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Creates a mesh of 1D duct cells around a triangular lattice subassembly");
  params.addRequiredParam<MeshGeneratorName>("input", "The corresponding subchannel mesh");
  params.addParam<unsigned int>("block_id", 2, "Domain Index");
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addParam<Real>("unheated_length_entry", 0.0, "Unheated length at entry [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addParam<Real>("unheated_length_exit", 0.0, "Unheated length at exit [m]");
  params.addRequiredParam<Real>("pitch", "Pitch [m]");
  params.addRequiredParam<unsigned int>("nrings", "Number of fuel Pin rings per assembly [-]");
  params.addRequiredParam<Real>("flat_to_flat",
                                "Flat to flat distance for the hexagonal assembly [m]");
  return params;
}

SCMTriDuctMeshGenerator::SCMTriDuctMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _n_cells(getParam<unsigned int>("n_cells")),
    _unheated_length_entry(getParam<Real>("unheated_length_entry")),
    _heated_length(getParam<Real>("heated_length")),
    _unheated_length_exit(getParam<Real>("unheated_length_exit")),
    _block_id(getParam<unsigned int>("block_id")),
    _pitch(getParam<Real>("pitch")),
    _n_rings(getParam<unsigned int>("nrings")),
    _flat_to_flat(getParam<Real>("flat_to_flat"))
{
  SubChannelMesh::generateZGrid(
      _unheated_length_entry, _heated_length, _unheated_length_exit, _n_cells, _z_grid);
}

std::unique_ptr<MeshBase>
SCMTriDuctMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh_base = std::move(_input);
  if (!mesh_base)
    mooseError("SCMTriDuctMeshGenerator has to be connected to a sub channel mesh generator.");

  mesh_base->set_mesh_dimension(3);

  std::vector<Point> corners;
  ductCorners(corners, _flat_to_flat, Point(0, 0, 0));
  std::vector<Point> xsec;
  ductXsec(xsec, corners, _n_rings, _pitch, _flat_to_flat);
  std::vector<Point> points;
  ductPoints(points, xsec, _z_grid);
  std::vector<std::vector<size_t>> elem_point_indices;
  ductElems(elem_point_indices, _z_grid.size(), xsec.size());
  std::vector<Node *> duct_nodes;
  buildDuct(mesh_base, duct_nodes, points, elem_point_indices, _block_id);
  mesh_base->subdomain_name(_block_id) = name();

  mesh_base->prepare_for_use();

  auto & sch_mesh = static_cast<TriSubChannelMesh &>(*_mesh);
  sch_mesh.setChannelToDuctMaps(duct_nodes);
  sch_mesh._duct_mesh_exist = true;

  return mesh_base;
}

void
SCMTriDuctMeshGenerator::ductCorners(std::vector<Point> & corners,
                                     Real flat_to_flat,
                                     const Point & center) const
{
  corners.resize(TriSubChannelMesh::N_CORNERS);
  Real r_corner = flat_to_flat / 2. / std::cos(libMesh::pi / 6.);
  for (size_t i = 0; i < TriSubChannelMesh::N_CORNERS; i++)
  {
    Real theta = i * libMesh::pi / 3.;
    Point corner = {r_corner * std::cos(theta), r_corner * std::sin(theta)};
    corners[i] = center + corner;
  }
}

void
SCMTriDuctMeshGenerator::ductXsec(std::vector<Point> & xsec,
                                  const std::vector<Point> & corners,
                                  unsigned int nrings,
                                  Real pitch,
                                  Real flat_to_flat) const
{
  xsec.clear();

  Real r_corner = flat_to_flat / 2. / std::cos(libMesh::pi / 6.);
  Real start_offset = (r_corner - (nrings - 2) * pitch) * std::sin(libMesh::pi / 6.);
  Real side_length = (corners[0] - corners[1]).norm();

  for (size_t i = 0; i < corners.size(); i++)
  {
    auto left = corners[i];
    auto right = corners[(i + 1) % corners.size()];
    xsec.push_back(left);
    auto direc = (right - left).unit();
    for (Real offset_from_corner = start_offset; offset_from_corner < side_length;
         offset_from_corner += pitch)
      xsec.push_back(left + direc * offset_from_corner);
  }
}

size_t
SCMTriDuctMeshGenerator::ductPointIndex(unsigned int points_per_layer,
                                        unsigned int layer,
                                        unsigned int point) const
{
  return layer * points_per_layer + point;
}

void
SCMTriDuctMeshGenerator::ductPoints(std::vector<Point> & points,
                                    const std::vector<Point> & xsec,
                                    const std::vector<Real> & z_layers) const
{
  points.resize(xsec.size() * z_layers.size());
  for (size_t i = 0; i < z_layers.size(); i++)
    for (size_t j = 0; j < xsec.size(); j++)
      points[ductPointIndex(xsec.size(), i, j)] = Point(xsec[j](0), xsec[j](1), z_layers[i]);
}

void
SCMTriDuctMeshGenerator::ductElems(std::vector<std::vector<size_t>> & elem_point_indices,
                                   unsigned int n_layers,
                                   unsigned int points_per_layer) const
{
  elem_point_indices.clear();
  for (unsigned int i = 0; i < n_layers - 1; i++)
  {
    unsigned int bottom = i;
    unsigned int top = i + 1;
    for (unsigned int j = 0; j < points_per_layer; j++)
    {
      unsigned int left = j;
      unsigned int right = (j + 1) % points_per_layer;
      elem_point_indices.push_back({ductPointIndex(points_per_layer, bottom, left),
                                    ductPointIndex(points_per_layer, bottom, right),
                                    ductPointIndex(points_per_layer, top, right),
                                    ductPointIndex(points_per_layer, top, left)});
    }
  }
}

void
SCMTriDuctMeshGenerator::buildDuct(std::unique_ptr<MeshBase> & mesh,
                                   std::vector<Node *> & duct_nodes,
                                   const std::vector<Point> & points,
                                   const std::vector<std::vector<size_t>> & elem_point_indices,
                                   SubdomainID block) const
{
  for (size_t i = 0; i < points.size(); i++)
    duct_nodes.push_back(mesh->add_point(points[i]));

  for (auto & elem_indices : elem_point_indices)
  {
    auto elem = mesh->add_elem(new Quad4());
    elem->subdomain_id() = block;
    for (size_t i = 0; i < elem_indices.size(); i++)
      elem->set_node(i, duct_nodes[elem_indices[i]]);
  }
}
