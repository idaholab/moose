//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OrientedBoundingBox.h"
#include <cmath>

#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/enum_elem_type.h"

namespace fs = std::filesystem;

OrientedBoundingBox::OrientedBoundingBox() = default;

OrientedBoundingBox::OrientedBoundingBox(const std::vector<std::pair<Point, Point>> & axis_pairs)
{
  _dim = axis_pairs.size();
  mooseAssert(_dim == 2 || _dim == 3, "OrientedBoundingBox requires 2 or 3 axis pairs");
  _dirs.resize(_dim);
  _len.resize(_dim);

  _minimal_corner = axis_pairs[0].first;

  _maximal_corner = _minimal_corner;
  // (a) Build orthonormal basis & lengths
  for (unsigned i = 0; i < _dim; ++i)
  {
    const Point vec = axis_pairs[i].second - _minimal_corner;
    _dirs[i] = vec.unit();
    _len[i] = vec.norm();
    _maximal_corner += _len[i] * _dirs[i];
  }

  // (b) Ensure orthogonality
  for (unsigned i = 0; i < _dim; ++i)
    for (unsigned j = i + 1; j < _dim; ++j)
      mooseAssert(MooseUtils::absoluteFuzzyEqual(_dirs[i] * _dirs[j], 0.0),
                  "Basis directions are not orthogonal");
}

void
OrientedBoundingBox::print(std::ostream & os) const
{
  os << "OrientedBoundingBox: dim=" << _dim << ", origin=" << _minimal_corner << '\n';
  for (unsigned i = 0; i < _dim; ++i)
    os << "  axis[" << i << "] dir=" << _dirs[i] << ", len=" << _len[i] << '\n';
}

bool
OrientedBoundingBox::contains(const Point & pt, const Real tolerance) const
{
  const Point rel = pt - _minimal_corner;
  for (unsigned i = 0; i < _dim; ++i)
  {
    const Real proj = rel * _dirs[i];
    if (!MooseUtils::absoluteFuzzyGreaterEqual(proj, 0.0, tolerance) ||
        !MooseUtils::absoluteFuzzyLessEqual(proj, _len[i], tolerance))
      return false;
  }
  return true;
}

Point
OrientedBoundingBox::centroid() const
{
  return 0.5 * (_minimal_corner + _maximal_corner);
}

Point
OrientedBoundingBox::getAxisDirection(unsigned int i) const
{
  mooseAssert(i < _dim, "Invalid axis index");
  return _dirs[i];
}

Real
OrientedBoundingBox::getAxisLength(unsigned int i) const
{
  mooseAssert(i < _dim, "Invalid axis index");
  return _len[i];
}

Real
OrientedBoundingBox::getProjectedLength(const Point & pt, unsigned int i) const
{
  mooseAssert(i < _dim, "Invalid axis index");
  const Point rel = pt - _minimal_corner;
  return rel * _dirs[i];
}

Point
OrientedBoundingBox::getMinimalCorner() const
{
  return _minimal_corner;
}

Point
OrientedBoundingBox::getMaximalCorner() const
{
  return _maximal_corner;
}

void
OrientedBoundingBox::writeMesh(const fs::path & path,
                               const libMesh::Parallel::Communicator & comm) const
{
  mooseAssert(_dim == 2 || _dim == 3, "writeMesh supports only 2D or 3D boxes.");

  // Build a single reference element (unit square in 2D, unit cube in 3D) and let
  // libMesh own the node ordering, connectivity, and output format.
  libMesh::ReplicatedMesh mesh(comm, _dim);
  if (_dim == 2)
    libMesh::MeshTools::Generation::build_square(mesh, 1, 1, 0., 1., 0., 1., libMesh::QUAD4);
  else
    libMesh::MeshTools::Generation::build_cube(
        mesh, 1, 1, 1, 0., 1., 0., 1., 0., 1., libMesh::HEX8);

  // Map each reference corner onto the oriented box:
  //   x = origin + sum_axis r_axis * len_axis * dir_axis
  for (auto * node : mesh.node_ptr_range())
  {
    const Point reference = *node;
    Point mapped = _minimal_corner;
    for (const auto axis : make_range(_dim))
      mapped += reference(axis) * _len[axis] * _dirs[axis];
    *node = mapped;
  }

  if (!path.parent_path().empty())
  {
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);
  }
  mesh.write(path.string());
}

void
OrientedBoundingBox::writeRayAlongShortestAxis(const fs::path & ray_path,
                                               const libMesh::Parallel::Communicator & comm) const
{
  mooseAssert(_dim == 2 || _dim == 3, "Ray writing only supports 2D or 3D");

  unsigned int i_min = 0;
  for (const auto i : make_range(1u, _dim))
    if (_len[i] < _len[i_min])
      i_min = i;

  Point start = _minimal_corner;
  for (const auto i : make_range(_dim))
    if (i != i_min)
      start += 0.5 * _len[i] * _dirs[i];

  const Point end = start + _len[i_min] * _dirs[i_min];

  // A single EDGE2 spanning the ray; libMesh owns the output format.
  libMesh::ReplicatedMesh mesh(comm, 1);
  libMesh::Node * n0 = mesh.add_point(start, 0);
  libMesh::Node * n1 = mesh.add_point(end, 1);
  auto edge = libMesh::Elem::build(libMesh::EDGE2);
  edge->set_node(0) = n0;
  edge->set_node(1) = n1;
  mesh.add_elem(std::move(edge));
  mesh.prepare_for_use();

  if (!ray_path.parent_path().empty())
  {
    std::error_code ec;
    fs::create_directories(ray_path.parent_path(), ec);
  }
  mesh.write(ray_path.string());
}
