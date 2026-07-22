//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OrientedBoundingBox.h"
#include <fstream>
#include <cmath>

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
OrientedBoundingBox::writeVTK(const fs::path & path) const
{
  mooseAssert(_dim == 2 || _dim == 3, "writeVTK supports only 2D or 3D boxes.");

  // Compute corner points
  std::vector<Point> corner;
  const unsigned int corner_count = 1u << _dim;
  for (unsigned mask = 0; mask < corner_count; ++mask)
  {
    Point p = _minimal_corner;
    for (unsigned axis = 0; axis < _dim; ++axis)
      if (mask & (1u << axis))
        p += _len[axis] * _dirs[axis];
    corner.push_back(p);
  }

  fs::create_directories(path.parent_path());
  std::ofstream out(path);
  mooseAssert(out.is_open(), "Unable to open '" + path.string() + "' for writing.");

  out << "# vtk DataFile Version 3.0\nOrientedBoundingBox\nASCII\n";
  out << "DATASET UNSTRUCTURED_GRID\n";
  out << "POINTS " << corner.size() << " float\n";
  for (const auto & p : corner)
    out << p(0) << ' ' << p(1) << ' ' << p(2) << '\n';

  if (_dim == 3)
  {
    static const unsigned face[6][4] = {
        {0, 1, 3, 2}, {4, 5, 7, 6}, {0, 1, 5, 4}, {1, 3, 7, 5}, {3, 2, 6, 7}, {2, 0, 4, 6}};

    out << "CELLS 6 30\n";
    for (const auto & f : face)
      out << "4 " << f[0] << ' ' << f[1] << ' ' << f[2] << ' ' << f[3] << '\n';

    out << "CELL_TYPES 6\n";
    for (int i = 0; i < 6; ++i)
      out << "9\n";
  }
  else
  {
    static const unsigned edge[4][2] = {{0, 1}, {1, 3}, {3, 2}, {2, 0}};

    out << "CELLS 4 12\n";
    for (const auto & e : edge)
      out << "2 " << e[0] << ' ' << e[1] << '\n';

    out << "CELL_TYPES 4\n";
    for (int i = 0; i < 4; ++i)
      out << "3\n";
  }
}

void
OrientedBoundingBox::writeRayAlongShortestAxis(const fs::path & ray_path) const
{
  mooseAssert(_dim == 2 || _dim == 3, "Ray writing only supports 2D or 3D");

  unsigned i_min = 0;
  for (unsigned i = 1; i < _dim; ++i)
    if (_len[i] < _len[i_min])
      i_min = i;

  Point start = _minimal_corner;
  for (unsigned i = 0; i < _dim; ++i)
    if (i != i_min)
      start += 0.5 * _len[i] * _dirs[i];

  const Point dir = _dirs[i_min];
  const Real ray_length = _len[i_min];
  const Point end = start + ray_length * dir;

  std::filesystem::create_directories(ray_path.parent_path());
  std::ofstream out(ray_path);
  mooseAssert(out.is_open(), "Unable to open '" + ray_path.string() + "' for writing.");

  out << "# vtk DataFile Version 3.0\nRay\nASCII\n";
  out << "DATASET POLYDATA\n";
  out << "POINTS 2 float\n";
  out << start(0) << ' ' << start(1) << ' ' << start(2) << '\n';
  out << end(0) << ' ' << end(1) << ' ' << end(2) << '\n';
  out << "LINES 1 3\n2 0 1\n";
}
