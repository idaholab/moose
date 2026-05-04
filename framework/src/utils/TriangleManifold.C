//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriangleManifold.h"

#include "GeometryUtils.h"
#include "MooseError.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/unstructured_mesh.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace TriangleManifoldUtils
{
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

class SurfaceChecker : public libMesh::MeshTetInterface
{
public:
  explicit SurfaceChecker(UnstructuredMesh & mesh) : libMesh::MeshTetInterface(mesh) {}
  void triangulate() override { mooseError("SurfaceChecker is not meant for triangulation."); }
  std::string improveAndValidate();
};
}

TriangleManifold::TriangleManifold(MeshBase & mesh, const Real surface_tolerance)
  : _mesh(mesh),
    _surface_tolerance(surface_tolerance),
    _bounding_box(MeshTools::create_bounding_box(_mesh)),
    _point_locator(_mesh.sub_point_locator())
{
  mooseAssert(_surface_tolerance > 0.0, "surface_tolerance must be strictly positive.");
  mooseAssert(mesh.is_serial(), "Input manifold mesh must be serialized.");
  mooseAssert(mesh.mesh_dimension() == 2, "Manifold mesh must be a surface.");

  // Finish topology validation and acceleration-structure setup before queries are allowed.
  finalize();

  _point_locator->set_close_to_point_tol(_surface_tolerance);
}

bool
TriangleManifold::contains(const Point & point) const
{
  // Most points are rejected here before we perform any per-triangle work.
  if (!pointInsideBoundingBox(point))
    return false;

  // By design, near-surface points count as inside for downstream subdomain tagging.
  if (pointOnSurface(point))
    return true;

  // Candidate filtering keeps the parity test from touching every triangle in large surfaces.
  const auto candidates = rayCandidates(point);
  unsigned int num_hits = 0;

  for (const auto triangle_index : candidates)
  {
    const auto tri = _mesh.elem_ptr(triangle_index);
    // Triangles wholly behind the ray origin cannot contribute to the parity count.
    if (tri->loose_bounding_box().max()(0) < point(0) - _surface_tolerance)
      continue;

    switch (rayIntersectsTriangle(point, *tri))
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

void
TriangleManifold::finalize()
{
  // Validate that the mesh can be used as a manifold
  // We use the same logic as determining if the mesh can the tetrahedralized.
  auto umesh = dynamic_cast<UnstructuredMesh *>(&_mesh);
  TriangleManifoldUtils::SurfaceChecker checker(*umesh);
  auto msg = checker.improveAndValidate();
  if (!msg.empty())
    mooseError(
        "The inputted surface mesh cannot be treated as manifold for the following reasons:\n",
        msg);

  buildCandidateGrid();
}

void
TriangleManifold::buildCandidateGrid()
{
  // Since every containment ray travels in +x, only y and z are needed to choose candidate
  // triangles for the parity test.
  const auto extent_y =
      std::max(boundingBox().max()(1) - boundingBox().min()(1), _surface_tolerance);
  const auto extent_z =
      std::max(boundingBox().max()(2) - boundingBox().min()(2), _surface_tolerance);

  // Choose a roughly square yz grid scaled by aspect ratio so candidate lists stay short.
  const auto target_cells =
      std::max<dof_id_type>(1, static_cast<dof_id_type>(std::sqrt(numTriangles())));
  const auto aspect = std::sqrt(extent_y / extent_z);

  _num_y_cells = std::max<dof_id_type>(
      1, static_cast<dof_id_type>(std::lround(std::sqrt(target_cells) * aspect)));
  _num_z_cells = std::max<std::size_t>(
      1, static_cast<std::size_t>(std::ceil(static_cast<Real>(target_cells) / _num_y_cells)));

  _y_min = boundingBox().min()(1);
  _z_min = boundingBox().min()(2);
  _y_cell_size = extent_y / _num_y_cells;
  _z_cell_size = extent_z / _num_z_cells;

  for (const auto elem : _mesh.active_element_ptr_range())
  {
    const auto triangle_index = elem->id();
    const auto bbox = elem->loose_bounding_box();

    const auto y_start =
        TriangleManifoldUtils::cellIndex(bbox.min()(1), _y_min, _y_cell_size, _num_y_cells);
    const auto y_stop =
        TriangleManifoldUtils::cellIndex(bbox.max()(1), _y_min, _y_cell_size, _num_y_cells);
    const auto z_start =
        TriangleManifoldUtils::cellIndex(bbox.min()(2), _z_min, _z_cell_size, _num_z_cells);
    const auto z_stop =
        TriangleManifoldUtils::cellIndex(bbox.max()(2), _z_min, _z_cell_size, _num_z_cells);

    // Insert the triangle into every grid cell touched by its yz projection.
    for (const auto iy : make_range(y_start, y_stop + 1))
      for (const auto iz : make_range(z_start, z_stop + 1))
        _ray_grid[TriangleManifoldUtils::packCell(iy, iz)].push_back(triangle_index);
  }
}

bool
TriangleManifold::pointInsideBoundingBox(const Point & point) const
{
  // Inflate the global box by the tolerance so near-surface points are not rejected too early.
  return point(0) >= boundingBox().min()(0) - _surface_tolerance &&
         point(0) <= boundingBox().max()(0) + _surface_tolerance &&
         point(1) >= boundingBox().min()(1) - _surface_tolerance &&
         point(1) <= boundingBox().max()(1) + _surface_tolerance &&
         point(2) >= boundingBox().min()(2) - _surface_tolerance &&
         point(2) <= boundingBox().max()(2) + _surface_tolerance;
}

bool
TriangleManifold::pointOnSurface(const Point & point) const
{
  return (*_point_locator)(point) != nullptr;
}

TriangleManifold::RayIntersection
TriangleManifold::rayIntersectsTriangle(const Point & point, const libMesh::Elem & tri) const
{
  // This is a fixed-direction Moller-Trumbore style ray/triangle intersection test.
  static const Point direction(1.0, 0.0, 0.0);

  const Point edge1 = tri.node_ref(1) - tri.node_ref(0);
  const Point edge2 = tri.node_ref(2) - tri.node_ref(0);
  const Point h = direction.cross(edge2);
  const Real determinant = edge1 * h;
  const Real characteristic_length = std::max(edge1.norm(), edge2.norm());

  if (std::abs(determinant) <= _surface_tolerance * characteristic_length)
    // Nearly parallel triangles are ignored because they do not provide a stable parity event.
    return RayIntersection::Miss;

  const Real inv_determinant = 1.0 / determinant;
  const Point s = point - tri.node_ref(0);
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
  if (geom_utils::pointSegmentDistanceSq(intersection, tri.node_ref(0), tri.node_ref(1)) <=
          tolerance_sq ||
      geom_utils::pointSegmentDistanceSq(intersection, tri.node_ref(1), tri.node_ref(2)) <=
          tolerance_sq ||
      geom_utils::pointSegmentDistanceSq(intersection, tri.node_ref(2), tri.node_ref(0)) <=
          tolerance_sq)
    // Edge and vertex hits are where odd/even parity counting is the least reliable.
    return RayIntersection::Ambiguous;

  return RayIntersection::Hit;
}

bool
TriangleManifold::containsBySolidAngle(const Point & point) const
{
  // For a closed oriented mesh, the total solid angle is approximately +-4\pi inside and 0
  // outside. Taking parity over components handles nested shells cleanly.
  Real total_angle = 0.0;
  for (const auto elem : _mesh.active_element_ptr_range())
    total_angle +=
        geom_utils::solidAngle(point, elem->node_ref(0), elem->node_ref(1), elem->node_ref(2));

  return std::abs(total_angle) > 2.0 * libMesh::pi;
}

std::vector<dof_id_type>
TriangleManifold::rayCandidates(const Point & point) const
{
  // If the query is outside the manifold's yz extent, the fixed +x ray cannot hit anything.
  if (point(1) < boundingBox().min()(1) - _surface_tolerance ||
      point(1) > boundingBox().max()(1) + _surface_tolerance ||
      point(2) < boundingBox().min()(2) - _surface_tolerance ||
      point(2) > boundingBox().max()(2) + _surface_tolerance)
    return {};

  const auto y_index =
      TriangleManifoldUtils::cellIndex(point(1), _y_min, _y_cell_size, _num_y_cells);
  const auto z_index =
      TriangleManifoldUtils::cellIndex(point(2), _z_min, _z_cell_size, _num_z_cells);
  const auto it = _ray_grid.find(TriangleManifoldUtils::packCell(y_index, z_index));
  if (it == _ray_grid.end())
    return {};

  // Return by value to keep the helper simple and avoid exposing internal storage.
  return it->second;
}

std::string
TriangleManifoldUtils::SurfaceChecker::improveAndValidate()
{
  using libMesh::MeshTetInterface;
  auto result = improve_hull_integrity();

  if (result.empty())
    return "";

  std::ostringstream err_msg;
  if (result.count(NON_TRI3))
    err_msg << "- At least one non-Tri3 element was found.\n" << std::endl;
  if (result.count(MISSING_NEIGHBOR))
    err_msg << "- At least one triangle without three neighbors was found.\n" << std::endl;
  if (result.count(EMPTY_MESH))
    err_msg << "- The surface mesh was empty\n" << std::endl;
  if (result.count(MISSING_BACKLINK))
    err_msg << "- At least one triangle neighbor without a return neighbor link was found.\n";
  if (result.count(BAD_NEIGHBOR_NODES))
    err_msg << "- At least one triangle neighbor without expected node links was found.\n";
  if (result.count(NON_ORIENTED))
    err_msg << "- At least one triangle neighbor with an inconsistent orientation was found.\n";
  if (result.count(BAD_NEIGHBOR_LINKS))
    err_msg << "- At least one triangle neighbor with inconsistent node and neighbor links was "
               "found.\n";
  if (result.count(DEGENERATE_ELEMENT))
    err_msg << "- At least one input triangle is degenerate, with near-zero area relative to the "
               "manifold.\n";
  if (result.count(DEGENERATE_MESH))
    err_msg << "- Mesh is degenerate, with zero thickness in at least one direction.\n";
  return err_msg.str();
}
