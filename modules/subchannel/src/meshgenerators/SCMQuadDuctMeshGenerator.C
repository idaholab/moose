//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMQuadDuctMeshGenerator.h"
#include "QuadSubChannelMesh.h"
#include <cmath>
#include "libmesh/unstructured_mesh.h"
#include "libmesh/face_quad4.h"

registerMooseObject("SubChannelApp", SCMQuadDuctMeshGenerator);

InputParameters
SCMQuadDuctMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("Creates a mesh of 1D duct cells around a square-lattice subassembly");
  params.addRequiredParam<MeshGeneratorName>("input", "The corresponding subchannel mesh");
  params.addParam<unsigned int>("block_id", 2, "Domain Index");
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addParam<Real>("unheated_length_entry", 0.0, "Unheated length at entry [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addParam<Real>("unheated_length_exit", 0.0, "Unheated length at exit [m]");
  params.addRequiredParam<Real>("pitch", "Pitch [m]");
  params.addRequiredParam<unsigned int>("nx", "Number of channels in the x direction [-]");
  params.addRequiredParam<unsigned int>("ny", "Number of channels in the y direction [-]");
  params.addRequiredParam<Real>("side_gap",
                                "Gap between duct wall and outer pin lattice: distance(edge pin "
                                "center, duct wall) = pitch/2 + side_gap [m]");
  return params;
}

SCMQuadDuctMeshGenerator::SCMQuadDuctMeshGenerator(const InputParameters & params)
  : MeshGenerator(params),
    _input(getMesh("input")),
    _n_cells(getParam<unsigned int>("n_cells")),
    _unheated_length_entry(getParam<Real>("unheated_length_entry")),
    _heated_length(getParam<Real>("heated_length")),
    _unheated_length_exit(getParam<Real>("unheated_length_exit")),
    _block_id(getParam<unsigned int>("block_id")),
    _pitch(getParam<Real>("pitch")),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _side_gap(getParam<Real>("side_gap"))
{
  SubChannelMesh::generateZGrid(
      _unheated_length_entry, _heated_length, _unheated_length_exit, _n_cells, _z_grid);

  if (_pitch <= 0.0)
    mooseError(name(), ": 'pitch' must be > 0.");

  if (_nx < 2 || _ny < 2)
    mooseError(name(), ": 'nx' and 'ny' must both be >= 2.");
}

std::unique_ptr<MeshBase>
SCMQuadDuctMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh_base = std::move(_input);
  if (!mesh_base)
    mooseError("SCMQuadDuctMeshGenerator has to be connected to a sub channel mesh generator.");

  mesh_base->set_mesh_dimension(3);

  std::vector<Point> xsec;
  ductXsec(xsec, _nx, _ny, _pitch, _side_gap);
  std::vector<Point> points;
  ductPoints(points, xsec, _z_grid);
  std::vector<std::vector<size_t>> elem_point_indices;
  ductElems(elem_point_indices, _z_grid.size(), xsec.size());
  std::vector<Node *> duct_nodes;
  buildDuct(mesh_base, duct_nodes, points, elem_point_indices, _block_id);
  mesh_base->subdomain_name(_block_id) = name();

  mesh_base->prepare_for_use();

  // Mirror the Tri variant: provide mapping hooks into the subchannel mesh
  auto & sch_mesh = static_cast<QuadSubChannelMesh &>(*_mesh);
  sch_mesh.setChannelToDuctMaps(duct_nodes);

  return mesh_base;
}

size_t
SCMQuadDuctMeshGenerator::ductPointIndex(unsigned int points_per_layer,
                                         unsigned int layer,
                                         unsigned int point) const
{
  return layer * points_per_layer + point;
}

void
SCMQuadDuctMeshGenerator::ductCorners(std::vector<Point> & corners,
                                      Real half_x,
                                      Real half_y,
                                      const Point & center) const
{
  corners.resize(4);
  corners[0] = center + Point(-half_x, -half_y, 0);
  corners[1] = center + Point(half_x, -half_y, 0);
  corners[2] = center + Point(half_x, half_y, 0);
  corners[3] = center + Point(-half_x, half_y, 0);
}

void
SCMQuadDuctMeshGenerator::ductXsec(
    std::vector<Point> & xsec, unsigned int nx, unsigned int ny, Real pitch, Real side_gap) const
{
  xsec.clear();

  const Real half_x = 0.5 * (nx - 1) * pitch + side_gap;
  const Real half_y = 0.5 * (ny - 1) * pitch + side_gap;

  // Subchannel-aligned grid extents (no side_gap)
  const Real x0 = -0.5 * (nx - 1) * pitch;
  const Real y0 = -0.5 * (ny - 1) * pitch;

  // ---- Bottom edge (y = -half_y): nx points, left -> right ----
  for (unsigned int i = 0; i < nx; ++i)
  {
    Real x;
    if (i == 0)
      x = -half_x;
    else if (i == nx - 1)
      x = half_x;
    else
      x = x0 + i * pitch; // aligned with subchannel x grid

    xsec.emplace_back(x, -half_y, 0.0);
  }

  // ---- Right edge (x = +half_x): (ny-2) points, bottom -> top (exclude corners) ----
  for (unsigned int j = 1; j + 1 < ny; ++j)
  {
    const Real y = y0 + j * pitch; // aligned with subchannel y grid
    xsec.emplace_back(half_x, y, 0.0);
  }

  // ---- Top edge (y = +half_y): nx points, right -> left ----
  for (unsigned int i = 0; i < nx; ++i)
  {
    const unsigned int ii = nx - 1 - i;

    Real x;
    if (ii == 0)
      x = -half_x;
    else if (ii == nx - 1)
      x = half_x;
    else
      x = x0 + ii * pitch;

    xsec.emplace_back(x, half_y, 0.0);
  }

  // ---- Left edge (x = -half_x): (ny-2) points, top -> bottom (exclude corners) ----
  for (unsigned int j = 1; j + 1 < ny; ++j)
  {
    const unsigned int jj = ny - 1 - j;
    const Real y = y0 + jj * pitch; // aligned with subchannel y grid
    xsec.emplace_back(-half_x, y, 0.0);
  }

  // Total points: 2*nx + 2*ny - 4
}

void
SCMQuadDuctMeshGenerator::ductPoints(std::vector<Point> & points,
                                     const std::vector<Point> & xsec,
                                     const std::vector<Real> & z_layers) const
{
  points.resize(xsec.size() * z_layers.size());
  for (size_t i = 0; i < z_layers.size(); i++)
    for (size_t j = 0; j < xsec.size(); j++)
      points[ductPointIndex(xsec.size(), i, j)] = Point(xsec[j](0), xsec[j](1), z_layers[i]);
}

void
SCMQuadDuctMeshGenerator::ductElems(std::vector<std::vector<size_t>> & elem_point_indices,
                                    unsigned int n_layers,
                                    unsigned int points_per_layer) const
{
  elem_point_indices.clear();
  for (unsigned int i = 0; i < n_layers - 1; i++)
  {
    const unsigned int bottom = i;
    const unsigned int top = i + 1;
    for (unsigned int j = 0; j < points_per_layer; j++)
    {
      const unsigned int left = j;
      const unsigned int right = (j + 1) % points_per_layer;
      elem_point_indices.push_back({ductPointIndex(points_per_layer, bottom, left),
                                    ductPointIndex(points_per_layer, bottom, right),
                                    ductPointIndex(points_per_layer, top, right),
                                    ductPointIndex(points_per_layer, top, left)});
    }
  }
}

void
SCMQuadDuctMeshGenerator::buildDuct(std::unique_ptr<MeshBase> & mesh,
                                    std::vector<Node *> & duct_nodes,
                                    const std::vector<Point> & points,
                                    const std::vector<std::vector<size_t>> & elem_point_indices,
                                    SubdomainID block) const
{
  duct_nodes.reserve(points.size());
  for (const auto & p : points)
    duct_nodes.push_back(mesh->add_point(p));

  for (const auto & elem_indices : elem_point_indices)
  {
    auto elem = mesh->add_elem(new Quad4());
    elem->subdomain_id() = block;
    for (unsigned int i = 0; i < elem_indices.size(); i++)
      elem->set_node(i, duct_nodes[elem_indices[i]]);
  }
}
