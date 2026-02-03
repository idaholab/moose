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

InputParameters
SCMTriDuctMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Creates a mesh of 2D duct cells around a triangular lattice subassembly");
  params.addRequiredParam<MeshGeneratorName>("input", "The corresponding subchannel mesh");
  params.addParam<unsigned int>("block_id", 2, "Domain Index");
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addParam<Real>("unheated_length_entry", 0.0, "Unheated length at entry [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addParam<Real>("unheated_length_exit", 0.0, "Unheated length at exit [m]");
  params.addRequiredParam<Real>("pitch", "Pitch is the distance between adjacent pins [m]");
  params.addRequiredParam<unsigned int>("nrings", "Number of fuel Pin rings in the assembly [-]");
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
  mesh_base->set_mesh_dimension(3);

  std::vector<Point> corners;
  ductCorners(corners, _flat_to_flat, Point(0, 0, 0));
  std::vector<Point> cross_sec;
  ductCrossSec(cross_sec, corners, _n_rings, _pitch);
  std::vector<Point> points;
  ductPoints(points, cross_sec, _z_grid);
  std::vector<std::vector<size_t>> elem_point_indices;
  ductElems(elem_point_indices, _z_grid.size(), cross_sec.size());
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
SCMTriDuctMeshGenerator::ductCrossSec(std::vector<Point> & cross_sec,
                                      const std::vector<Point> & corners,
                                      unsigned int nrings,
                                      Real pitch) const
{
  cross_sec.clear();

  if (pitch <= 0.0)
    mooseError("The 'pitch' must be > 0.");

  if (nrings < 2)
    mooseError("The 'nrings' must be >= 2.");

  // Side length of the regular hexagon (distance between adjacent corners)
  const Real side_length = (corners[0] - corners[1]).norm();

  // start_offset = (hexagon_side - (nrings - 2)*pitch) / 2
  const Real start_offset = 0.5 * (side_length - (nrings - 2) * pitch);

  // If this is negative, the requested layout cannot fit.
  if (start_offset < 0.0)
    mooseError("SCMTriDuctMeshGenerator: computed start_offset is negative (",
               start_offset,
               "). Check 'nrings', 'pitch', and duct side length.");

  for (size_t i = 0; i < corners.size(); i++)
  {
    const Point left = corners[i];
    const Point right = corners[(i + 1) % corners.size()];

    // Always include the left corner
    cross_sec.push_back(left);

    // Unit direction along this side
    const Point direc = (right - left).unit();

    // Add exactly (nrings - 1) points after the left corner
    // at start_offset + k*pitch, k = 0..(nrings-2)
    for (const auto k : make_range(nrings - 1))
    {
      const Real offset_from_corner = start_offset + k * pitch;
      cross_sec.push_back(left + direc * offset_from_corner);
    }
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
                                    const std::vector<Point> & cross_sec,
                                    const std::vector<Real> & z_layers) const
{
  points.resize(cross_sec.size() * z_layers.size());
  for (size_t i = 0; i < z_layers.size(); i++)
    for (size_t j = 0; j < cross_sec.size(); j++)
      points[ductPointIndex(cross_sec.size(), i, j)] =
          Point(cross_sec[j](0), cross_sec[j](1), z_layers[i]);
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
  // Create mesh nodes for all duct points and keep a local index -> Node* map
  duct_nodes.clear();
  duct_nodes.reserve(points.size());
  for (const auto & p : points)
    duct_nodes.push_back(mesh->add_point(p));

  // Create QUAD4 surface elements using libMesh factory style
  for (const auto & elem_indices : elem_point_indices)
  {
    mooseAssert(elem_indices.size() == 4,
                "Expected 4 node indices per element when building QUAD4 elements.");

    auto elem = Elem::build(ElemType::QUAD4);
    elem->subdomain_id() = block;

    // Set the 4 nodes of the QUAD4
    for (unsigned int i = 0; i < 4; ++i)
    {
      const auto idx = elem_indices[i];
      mooseAssert(idx < duct_nodes.size(), "Element node index out of range.");
      elem->set_node(i, duct_nodes[idx]);
    }

    // Hand ownership to the mesh
    mesh->add_elem(elem.release());
  }
}
