//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "STLManifold.h"

#include "MooseError.h"
#include "MooseUtils.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/int_range.h"
#include "libmesh/tensor_value.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <numeric>
#include <queue>
#include <sstream>

namespace
{
using namespace libMesh;

// Integerized point used to make the topological manifold check tolerant to tiny
// floating-point coordinate noise.
struct QuantizedPoint
{
  std::array<long long, 3> values;

  bool operator==(const QuantizedPoint & other) const { return values == other.values; }
  bool operator<(const QuantizedPoint & other) const { return values < other.values; }
};

struct QuantizedPointHash
{
  std::size_t operator()(const QuantizedPoint & point) const
  {
    // Combine the quantized coordinates into one stable hash key for unordered
    // containers used during topology bookkeeping.
    std::size_t seed = 0;
    for (const auto value : point.values)
      libMesh::boostcopy::hash_combine(seed, value);
    return seed;
  }
};

struct EdgeKey
{
  // Canonically ordered first endpoint.
  QuantizedPoint a;

  // Canonically ordered second endpoint.
  QuantizedPoint b;

  bool operator==(const EdgeKey & other) const { return a == other.a && b == other.b; }
};

struct EdgeKeyHash
{
  std::size_t operator()(const EdgeKey & edge) const
  {
    std::size_t seed = QuantizedPointHash{}(edge.a);
    libMesh::boostcopy::hash_combine(seed, QuantizedPointHash{}(edge.b));
    return seed;
  }
};

struct EdgeOccurrence
{
  // Triangle that owns this edge occurrence.
  std::size_t triangle;

  // Whether the triangle traverses the edge in the same direction as the canonical sorted order.
  bool same_direction_as_sorted;
};

QuantizedPoint
quantizePoint(const Point & point, const Real tolerance)
{
  // The tolerance defines the spatial resolution of the topological connectivity test.
  return {{std::llround(point(0) / tolerance),
           std::llround(point(1) / tolerance),
           std::llround(point(2) / tolerance)}};
}

EdgeKey
makeEdgeKey(const QuantizedPoint & p0, const QuantizedPoint & p1)
{
  // Store every undirected edge under one canonical ordering.
  if (p1 < p0)
    return {p1, p0};
  return {p0, p1};
}

bool
sameDirectionAsSorted(const QuantizedPoint & p0, const QuantizedPoint & p1)
{
  // Preserve the original edge orientation relative to the canonical ordering.
  return !(p1 < p0);
}

Point
triangleMinPoint(const Point & a, const Point & b, const Point & c)
{
  return {std::min({a(0), b(0), c(0)}), std::min({a(1), b(1), c(1)}), std::min({a(2), b(2), c(2)})};
}

Point
triangleMaxPoint(const Point & a, const Point & b, const Point & c)
{
  return {std::max({a(0), b(0), c(0)}), std::max({a(1), b(1), c(1)}), std::max({a(2), b(2), c(2)})};
}

std::uint64_t
packCell(const std::size_t y_index, const std::size_t z_index)
{
  // The acceleration structure is indexed in yz because all rays travel in the +x direction.
  return (static_cast<std::uint64_t>(y_index) << 32) | static_cast<std::uint64_t>(z_index);
}

std::size_t
cellIndex(const Real value, const Real min_value, const Real cell_size, const std::size_t num_cells)
{
  // Clamp to the valid range so near-boundary roundoff does not produce an invalid cell id.
  if (num_cells <= 1)
    return 0;

  const auto index = static_cast<long long>(std::floor((value - min_value) / cell_size));
  if (index < 0)
    return 0;
  if (index >= static_cast<long long>(num_cells))
    return num_cells - 1;
  return static_cast<std::size_t>(index);
}
}

STLManifold::STLTriangle::STLTriangle(const Point & vertex0,
                                      const Point & vertex1,
                                      const Point & vertex2)
  : v0(vertex0),
    v1(vertex1),
    v2(vertex2),
    bbox(triangleMinPoint(vertex0, vertex1, vertex2), triangleMaxPoint(vertex0, vertex1, vertex2))
{
}

STLManifold::STLManifold(const std::string & file_name,
                         const RealVectorValue & scale,
                         const RealVectorValue & rotation,
                         const RealVectorValue & translation,
                         const Real surface_tolerance)
  : _file_name(file_name), _surface_tolerance(surface_tolerance)
{
  // A non-positive tolerance would make both quantization and surface classification ill-defined.
  if (_surface_tolerance <= 0.0)
    mooseError("surface_tolerance must be strictly positive.");

  // The current implementation accepts only strictly positive scale factors so the geometry is not
  // collapsed or mirrored unexpectedly.
  for (const auto i : make_range(Moose::dim))
    if (scale(i) <= 0.0)
      mooseError("scale components must be strictly positive.");

  // Parse while applying the requested scale -> rotation -> translation transform.
  parse(file_name, scale, rotation, translation);

  // Finish topology validation and acceleration-structure setup before queries are allowed.
  finalize();
}

bool
STLManifold::contains(const Point & point) const
{
  // Most points are rejected here before we perform any per-triangle work.
  if (!pointInsideBoundingBox(point))
    return false;

  // By design, near-surface points count as inside for downstream subdomain tagging.
  if (pointOnSurface(point))
    return true;

  // Candidate filtering keeps the parity test from touching every triangle in large STL files.
  const auto candidates = rayCandidates(point);
  unsigned int num_hits = 0;

  for (const auto triangle_index : candidates)
  {
    const auto & tri = _triangles[triangle_index];
    // Triangles wholly behind the ray origin cannot contribute to the parity count.
    if (tri.bbox.max()(0) < point(0) - _surface_tolerance)
      continue;

    switch (rayIntersectsTriangle(point, tri))
    {
      case RayIntersection::Miss:
        break;
      case RayIntersection::Hit:
        // Standard odd/even counting on a closed surface.
        ++num_hits;
        break;
      case RayIntersection::Ambiguous:
        // Edge and vertex grazing hits are exactly where parity counting becomes brittle.
        return containsBySolidAngle(point);
    }
  }

  return num_hits % 2;
}

const libMesh::BoundingBox &
STLManifold::boundingBox() const
{
  return *_bounding_box;
}

std::size_t
STLManifold::numTriangles() const
{
  return _triangles.size();
}

void
STLManifold::parse(const std::string & file_name,
                   const RealVectorValue & scale,
                   const RealVectorValue & rotation,
                   const RealVectorValue & translation)
{
  // Verify readability before attempting format detection.
  MooseUtils::checkFileReadable(file_name);

  std::ifstream probe(file_name, std::ios::binary);
  if (!probe)
    mooseError("Unable to open STL file '", file_name, "'.");

  probe.seekg(0, std::ios::end);
  const auto file_size = static_cast<std::size_t>(probe.tellg());
  probe.seekg(0, std::ios::beg);

  std::array<char, 84> header = {{0}};
  probe.read(header.data(), header.size());
  if (probe.gcount() < static_cast<std::streamsize>(header.size()) && file_size >= header.size())
    mooseError("Failed while reading STL file '", file_name, "'.");

  // Binary STL encodes a triangle count in bytes 80-83, so the expected file size is known.
  bool parse_as_binary = false;
  if (file_size >= 84)
  {
    std::uint32_t triangle_count = 0;
    std::memcpy(&triangle_count, header.data() + 80, sizeof(std::uint32_t));
    const auto expected_size = static_cast<std::size_t>(84ull + 50ull * triangle_count);
    parse_as_binary = triangle_count > 0 && expected_size == file_size;
  }

  probe.close();

  if (parse_as_binary)
  {
    std::ifstream input(file_name, std::ios::binary);
    parseBinary(input, scale, rotation, translation);
  }
  else
  {
    // ASCII STL is the fallback because some binary STL headers begin with "solid" as well.
    std::ifstream input(file_name);
    parseASCII(input, scale, rotation, translation);
  }
}

void
STLManifold::parseASCII(std::istream & input,
                        const RealVectorValue & scale,
                        const RealVectorValue & rotation,
                        const RealVectorValue & translation)
{
  if (!input)
    mooseError("Unable to open STL file '", _file_name, "'.");

  // Ignore stored facet normals; geometric operations are driven by vertices only.
  const auto rotation_matrix =
      RealTensorValue::extrinsic_rotation_matrix(rotation(0), rotation(1), rotation(2));
  const Point translation_point(translation(0), translation(1), translation(2));

  std::string token;
  std::vector<Point> vertices;
  while (input >> token)
    if (token == "vertex")
    {
      Real x = 0.0;
      Real y = 0.0;
      Real z = 0.0;
      if (!(input >> x >> y >> z))
        mooseError("Failed parsing ASCII STL file '", _file_name, "'.");
      vertices.emplace_back(x, y, z);
    }

  if (vertices.size() % 3 != 0)
    mooseError("ASCII STL file '", _file_name, "' does not contain complete triangles.");

  // STL stores independent triangles, so vertices are consumed in groups of three.
  for (std::size_t i = 0; i < vertices.size(); i += 3)
    addTriangle(
        vertices[i], vertices[i + 1], vertices[i + 2], scale, rotation_matrix, translation_point);
}

void
STLManifold::parseBinary(std::istream & input,
                         const RealVectorValue & scale,
                         const RealVectorValue & rotation,
                         const RealVectorValue & translation)
{
  if (!input)
    mooseError("Unable to open STL file '", _file_name, "'.");

  const auto rotation_matrix =
      RealTensorValue::extrinsic_rotation_matrix(rotation(0), rotation(1), rotation(2));
  const Point translation_point(translation(0), translation(1), translation(2));

  std::array<char, 80> header;
  input.read(header.data(), header.size());

  std::uint32_t triangle_count = 0;
  input.read(reinterpret_cast<char *>(&triangle_count), sizeof(std::uint32_t));
  if (!input)
    mooseError("Failed parsing binary STL file '", _file_name, "'.");

  for (std::uint32_t triangle_index = 0; triangle_index < triangle_count; ++triangle_index)
  {
    // Binary STL stores one normal, three vertices, and a two-byte attribute count per triangle.
    float facet_data[12] = {0.0f};
    input.read(reinterpret_cast<char *>(facet_data), sizeof(facet_data));

    std::uint16_t attribute_count = 0;
    input.read(reinterpret_cast<char *>(&attribute_count), sizeof(std::uint16_t));
    libmesh_ignore(attribute_count);

    if (!input)
      mooseError("Failed parsing binary STL file '", _file_name, "'.");

    addTriangle(Point(facet_data[3], facet_data[4], facet_data[5]),
                Point(facet_data[6], facet_data[7], facet_data[8]),
                Point(facet_data[9], facet_data[10], facet_data[11]),
                scale,
                rotation_matrix,
                translation_point);
  }
}

void
STLManifold::finalize()
{
  // Empty STL input can never define a useful solid region.
  if (_triangles.empty())
    mooseError("STL file '", _file_name, "' does not contain any triangles.");

  // Topology must be valid before geometric acceleration structures are meaningful.
  validateAndOrient();
  buildCandidateGrid();
}

void
STLManifold::validateAndOrient()
{
  // The edge map is the heart of the manifold check: a watertight manifold has each undirected
  // edge shared by exactly two triangles.
  std::unordered_map<EdgeKey, std::vector<EdgeOccurrence>, EdgeKeyHash> edge_map;
  std::vector<std::vector<std::pair<std::size_t, bool>>> adjacency(_triangles.size());

  for (const auto tri_index : make_range(_triangles.size()))
  {
    const auto & tri = _triangles[tri_index];

    // Quantize vertices so tiny roundoff differences do not split what should be one shared edge.
    const std::array<QuantizedPoint, 3> points = {quantizePoint(tri.v0, _surface_tolerance),
                                                  quantizePoint(tri.v1, _surface_tolerance),
                                                  quantizePoint(tri.v2, _surface_tolerance)};

    const auto register_edge =
        [&edge_map, tri_index](const QuantizedPoint & p0, const QuantizedPoint & p1)
    { edge_map[makeEdgeKey(p0, p1)].push_back({tri_index, sameDirectionAsSorted(p0, p1)}); };

    register_edge(points[0], points[1]);
    register_edge(points[1], points[2]);
    register_edge(points[2], points[0]);
  }

  for (const auto & [edge, occurrences] : edge_map)
  {
    libmesh_ignore(edge);
    // Open surfaces and non-manifold junctions both fail this exact-two-incidences requirement.
    if (occurrences.size() != 2)
      mooseError("STL file '",
                 _file_name,
                 "' must define a watertight 2-manifold surface. Each edge must be shared by "
                 "exactly two triangles.");

    const auto & first = occurrences[0];
    const auto & second = occurrences[1];

    // If both triangles traverse the shared edge in the same direction, one of them must be
    // flipped relative to the other to obtain a globally consistent orientation.
    const bool equal_direction = first.same_direction_as_sorted == second.same_direction_as_sorted;

    adjacency[first.triangle].push_back({second.triangle, equal_direction});
    adjacency[second.triangle].push_back({first.triangle, equal_direction});
  }

  std::vector<int> flip(_triangles.size(), -1);
  std::queue<std::size_t> queue;

  for (const auto triangle_index : make_range(_triangles.size()))
    if (flip[triangle_index] == -1)
    {
      // Run a BFS over each connected surface component and decide which triangles must be flipped.
      flip[triangle_index] = 0;
      queue.push(triangle_index);
      _components.emplace_back();

      while (!queue.empty())
      {
        const auto current = queue.front();
        queue.pop();
        _components.back().push_back(current);

        for (const auto & [neighbor, equal_direction] : adjacency[current])
        {
          // Neighbor orientation is determined entirely by the shared-edge direction relationship.
          const auto needed_flip = flip[current] ^ (equal_direction ? 1 : 0);
          if (flip[neighbor] == -1)
          {
            flip[neighbor] = needed_flip;
            queue.push(neighbor);
          }
          else if (flip[neighbor] != needed_flip)
            // Contradictory orientation requirements imply a non-orientable surface.
            mooseError("STL file '", _file_name, "' defines a non-orientable manifold surface.");
        }
      }
    }

  for (const auto triangle_index : make_range(_triangles.size()))
    if (flip[triangle_index])
      // Swapping two vertices reverses triangle winding without changing the geometric triangle.
      std::swap(_triangles[triangle_index].v1, _triangles[triangle_index].v2);
}

void
STLManifold::buildCandidateGrid()
{
  // Since every containment ray travels in +x, only y and z are needed to choose candidate
  // triangles for the parity test.
  const auto extent_y =
      std::max(_bounding_box->max()(1) - _bounding_box->min()(1), _surface_tolerance);
  const auto extent_z =
      std::max(_bounding_box->max()(2) - _bounding_box->min()(2), _surface_tolerance);

  // Choose a roughly square yz grid scaled by aspect ratio so candidate lists stay short.
  const auto target_cells =
      std::max<std::size_t>(1, static_cast<std::size_t>(std::sqrt(_triangles.size())));
  const auto aspect = std::sqrt(extent_y / extent_z);

  _num_y_cells = std::max<std::size_t>(
      1, static_cast<std::size_t>(std::lround(std::sqrt(target_cells) * aspect)));
  _num_z_cells = std::max<std::size_t>(
      1, static_cast<std::size_t>(std::ceil(static_cast<Real>(target_cells) / _num_y_cells)));

  _y_min = _bounding_box->min()(1);
  _z_min = _bounding_box->min()(2);
  _y_cell_size = extent_y / _num_y_cells;
  _z_cell_size = extent_z / _num_z_cells;

  for (const auto triangle_index : make_range(_triangles.size()))
  {
    const auto & tri = _triangles[triangle_index];

    const auto y_start = cellIndex(tri.bbox.min()(1), _y_min, _y_cell_size, _num_y_cells);
    const auto y_stop = cellIndex(tri.bbox.max()(1), _y_min, _y_cell_size, _num_y_cells);
    const auto z_start = cellIndex(tri.bbox.min()(2), _z_min, _z_cell_size, _num_z_cells);
    const auto z_stop = cellIndex(tri.bbox.max()(2), _z_min, _z_cell_size, _num_z_cells);

    // Insert the triangle into every grid cell touched by its yz projection.
    for (const auto iy : make_range(y_start, y_stop + 1))
      for (const auto iz : make_range(z_start, z_stop + 1))
        _ray_grid[packCell(iy, iz)].push_back(triangle_index);
  }
}

void
STLManifold::addTriangle(const Point & v0,
                         const Point & v1,
                         const Point & v2,
                         const RealVectorValue & scale,
                         const RealTensorValue & rotation_matrix,
                         const Point & translation)
{
  // Apply transforms in the promised order: scale, then rotate, then translate.
  const Point sv0(v0(0) * scale(0), v0(1) * scale(1), v0(2) * scale(2));
  const Point sv1(v1(0) * scale(0), v1(1) * scale(1), v1(2) * scale(2));
  const Point sv2(v2(0) * scale(0), v2(1) * scale(1), v2(2) * scale(2));

  const Point tv0 = rotation_matrix * sv0 + translation;
  const Point tv1 = rotation_matrix * sv1 + translation;
  const Point tv2 = rotation_matrix * sv2 + translation;

  // Reject triangles that are collapsed or nearly collapsed after transformation.
  const auto e0 = (tv1 - tv0).norm();
  const auto e1 = (tv2 - tv1).norm();
  const auto e2 = (tv0 - tv2).norm();
  const auto max_edge = std::max({e0, e1, e2});
  const auto area_measure = ((tv1 - tv0).cross(tv2 - tv0)).norm();

  if (e0 <= _surface_tolerance || e1 <= _surface_tolerance || e2 <= _surface_tolerance ||
      area_measure <= _surface_tolerance * max_edge)
    mooseError("STL file '", _file_name, "' contains a degenerate triangle.");

  _triangles.emplace_back(tv0, tv1, tv2);

  if (!_bounding_box)
    _bounding_box = std::make_unique<libMesh::BoundingBox>(_triangles.back().bbox);
  else
    // Update the global bounds incrementally as triangles are added.
    _bounding_box = std::make_unique<libMesh::BoundingBox>(
        Point(std::min(_bounding_box->min()(0), _triangles.back().bbox.min()(0)),
              std::min(_bounding_box->min()(1), _triangles.back().bbox.min()(1)),
              std::min(_bounding_box->min()(2), _triangles.back().bbox.min()(2))),
        Point(std::max(_bounding_box->max()(0), _triangles.back().bbox.max()(0)),
              std::max(_bounding_box->max()(1), _triangles.back().bbox.max()(1)),
              std::max(_bounding_box->max()(2), _triangles.back().bbox.max()(2))));
}

bool
STLManifold::pointInsideBoundingBox(const Point & point) const
{
  // Inflate the global box by the tolerance so near-surface points are not rejected too early.
  return point(0) >= _bounding_box->min()(0) - _surface_tolerance &&
         point(0) <= _bounding_box->max()(0) + _surface_tolerance &&
         point(1) >= _bounding_box->min()(1) - _surface_tolerance &&
         point(1) <= _bounding_box->max()(1) + _surface_tolerance &&
         point(2) >= _bounding_box->min()(2) - _surface_tolerance &&
         point(2) <= _bounding_box->max()(2) + _surface_tolerance;
}

bool
STLManifold::pointOnSurface(const Point & point) const
{
  const auto tolerance_sq = _surface_tolerance * _surface_tolerance;
  for (const auto & tri : _triangles)
  {
    // Use the cached triangle box as a cheap broad-phase filter before computing point-triangle
    // distance.
    if (point(0) < tri.bbox.min()(0) - _surface_tolerance ||
        point(0) > tri.bbox.max()(0) + _surface_tolerance ||
        point(1) < tri.bbox.min()(1) - _surface_tolerance ||
        point(1) > tri.bbox.max()(1) + _surface_tolerance ||
        point(2) < tri.bbox.min()(2) - _surface_tolerance ||
        point(2) > tri.bbox.max()(2) + _surface_tolerance)
      continue;

    if (pointTriangleDistanceSq(point, tri) <= tolerance_sq)
      return true;
  }

  return false;
}

STLManifold::RayIntersection
STLManifold::rayIntersectsTriangle(const Point & point, const STLTriangle & tri) const
{
  // This is a fixed-direction Moller-Trumbore style ray/triangle intersection test.
  static const Point direction(1.0, 0.0, 0.0);

  const Point edge1 = tri.v1 - tri.v0;
  const Point edge2 = tri.v2 - tri.v0;
  const Point h = direction.cross(edge2);
  const Real determinant = edge1 * h;

  if (std::abs(determinant) <= _surface_tolerance)
    // Nearly parallel triangles are ignored because they do not provide a stable parity event.
    return RayIntersection::Miss;

  const Real inv_determinant = 1.0 / determinant;
  const Point s = point - tri.v0;
  const Real u = inv_determinant * (s * h);
  if (u < 0.0 || u > 1.0)
    return RayIntersection::Miss;

  const Point q = s.cross(edge1);
  const Real v = inv_determinant * (direction * q);
  if (v < 0.0 || u + v > 1.0)
    return RayIntersection::Miss;

  const Real t = inv_determinant * (edge2 * q);
  if (t < 0.0)
    // Intersections behind the ray origin do not contribute to +x parity counting.
    return RayIntersection::Miss;
  if (t <= _surface_tolerance)
    // Hits too close to the ray origin are treated as ambiguous boundary situations.
    return RayIntersection::Ambiguous;

  const Point intersection = point + t * direction;
  const auto tolerance_sq = _surface_tolerance * _surface_tolerance;
  if (pointSegmentDistanceSq(intersection, tri.v0, tri.v1) <= tolerance_sq ||
      pointSegmentDistanceSq(intersection, tri.v1, tri.v2) <= tolerance_sq ||
      pointSegmentDistanceSq(intersection, tri.v2, tri.v0) <= tolerance_sq)
    // Edge and vertex hits are where odd/even parity counting is the least reliable.
    return RayIntersection::Ambiguous;

  return RayIntersection::Hit;
}

bool
STLManifold::containsBySolidAngle(const Point & point) const
{
  unsigned int num_inside_components = 0;
  for (const auto & component : _components)
  {
    // For a closed oriented component, the total solid angle is approximately +-4\pi inside and 0
    // outside. Taking parity over components handles nested shells cleanly.
    Real total_angle = 0.0;
    for (const auto triangle_index : component)
      total_angle += solidAngle(point, _triangles[triangle_index]);

    if (std::abs(total_angle) > 2.0 * libMesh::pi)
      ++num_inside_components;
  }

  return num_inside_components % 2;
}

std::vector<std::size_t>
STLManifold::rayCandidates(const Point & point) const
{
  // If the query is outside the manifold's yz extent, the fixed +x ray cannot hit anything.
  if (point(1) < _bounding_box->min()(1) - _surface_tolerance ||
      point(1) > _bounding_box->max()(1) + _surface_tolerance ||
      point(2) < _bounding_box->min()(2) - _surface_tolerance ||
      point(2) > _bounding_box->max()(2) + _surface_tolerance)
    return {};

  const auto y_index = cellIndex(point(1), _y_min, _y_cell_size, _num_y_cells);
  const auto z_index = cellIndex(point(2), _z_min, _z_cell_size, _num_z_cells);
  const auto it = _ray_grid.find(packCell(y_index, z_index));
  if (it == _ray_grid.end())
    return {};

  // Return by value to keep the helper simple and avoid exposing internal storage.
  return it->second;
}

Real
STLManifold::pointTriangleDistanceSq(const Point & point, const STLTriangle & tri) const
{
  // Region-based closest-point-on-triangle algorithm from Real-Time Collision Detection.
  const Point ab = tri.v1 - tri.v0;
  const Point ac = tri.v2 - tri.v0;
  const Point ap = point - tri.v0;
  const Real d1 = ab * ap;
  const Real d2 = ac * ap;
  if (d1 <= 0.0 && d2 <= 0.0)
    return (point - tri.v0).norm_sq();

  const Point bp = point - tri.v1;
  const Real d3 = ab * bp;
  const Real d4 = ac * bp;
  if (d3 >= 0.0 && d4 <= d3)
    return (point - tri.v1).norm_sq();

  const Real vc = d1 * d4 - d3 * d2;
  if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0)
  {
    const Real v = d1 / (d1 - d3);
    const Point projection = tri.v0 + v * ab;
    return (point - projection).norm_sq();
  }

  const Point cp = point - tri.v2;
  const Real d5 = ab * cp;
  const Real d6 = ac * cp;
  if (d6 >= 0.0 && d5 <= d6)
    return (point - tri.v2).norm_sq();

  const Real vb = d5 * d2 - d1 * d6;
  if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0)
  {
    const Real w = d2 / (d2 - d6);
    const Point projection = tri.v0 + w * ac;
    return (point - projection).norm_sq();
  }

  const Real va = d3 * d6 - d5 * d4;
  if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0)
  {
    const Point bc = tri.v2 - tri.v1;
    const Real w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
    const Point projection = tri.v1 + w * bc;
    return (point - projection).norm_sq();
  }

  const Real denom = 1.0 / (va + vb + vc);
  const Real v = vb * denom;
  const Real w = vc * denom;
  const Point projection = tri.v0 + ab * v + ac * w;
  return (point - projection).norm_sq();
}

Real
STLManifold::pointSegmentDistanceSq(const Point & point, const Point & a, const Point & b) const
{
  const Point ab = b - a;
  const auto length_sq = ab.norm_sq();
  if (length_sq <= std::numeric_limits<Real>::epsilon())
    // Degenerate segments are treated as a single endpoint.
    return (point - a).norm_sq();

  const auto t = std::clamp(((point - a) * ab) / length_sq, 0.0, 1.0);
  const Point projection = a + t * ab;
  return (point - projection).norm_sq();
}

Real
STLManifold::solidAngle(const Point & point, const STLTriangle & tri) const
{
  // Compute the signed solid angle subtended by one oriented triangle at the query point.
  const Point a = tri.v0 - point;
  const Point b = tri.v1 - point;
  const Point c = tri.v2 - point;

  const Real la = a.norm();
  const Real lb = b.norm();
  const Real lc = c.norm();

  const Real numerator = a * (b.cross(c));
  const Real denominator = la * lb * lc + (a * b) * lc + (b * c) * la + (c * a) * lb;

  return 2.0 * std::atan2(numerator, denominator);
}
