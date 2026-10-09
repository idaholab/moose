//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurfaceMeshContactor.h"

#include "KDTree.h"
#include "TriangleManifold.h"

#include "MooseUtils.h"

#include "libmesh/elem.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/replicated_mesh.h"

#include <algorithm>
#include <cmath>

registerMooseObject("ContactApp", SurfaceMeshContactor);

InputParameters
SurfaceMeshContactor::validParams()
{
  InputParameters params = LevelSetContactor::validParams();
  params.addClassDescription("Rigid contactor described by a closed, oriented, triangulated "
                             "surface mesh (e.g. STL).  Provides pointwise signed distance and "
                             "outward normal for use with RigidBodyNodalNCPKernel and "
                             "RigidBodyNormalMechanicalContact.");
  params.addRequiredParam<FileName>(
      "file",
      "Surface mesh file.  Format is dispatched from the extension via libMesh's mesh readers "
      "(e.g. .stl, .e).  Must be a closed, consistently outward-oriented 2-manifold of Tri3 "
      "elements after read+transform.");
  params.addParam<Point>("translation", Point(0, 0, 0), "Applied after scaling.");
  params.addRangeCheckedParam<Real>(
      "scale", 1.0, "scale > 0", "Uniform scale applied to the read mesh before translation.");
  params.addRangeCheckedParam<Real>(
      "surface_tolerance",
      1e-8,
      "surface_tolerance > 0",
      "Absolute tolerance used to validate the mesh (via TriangleManifold) at load time and "
      "to detect on-surface queries.  Choose relative to the mesh length scale.");
  return params;
}

SurfaceMeshContactor::SurfaceMeshContactor(const InputParameters & p)
  : LevelSetContactor(p),
    _file(getParam<FileName>("file")),
    _translation(getParam<Point>("translation")),
    _scale(getParam<Real>("scale")),
    _surface_tolerance(getParam<Real>("surface_tolerance"))
{
}

SurfaceMeshContactor::~SurfaceMeshContactor() = default;

void
SurfaceMeshContactor::initialSetup()
{
  // Serial (replicated) mesh so every rank has the full surface for pointwise queries.
  _mesh = std::make_unique<libMesh::ReplicatedMesh>(_communicator);
  _mesh->set_mesh_dimension(2);
  _mesh->read(_file);

  if (_scale != 1.0)
    libMesh::MeshTools::Modification::scale(*_mesh, _scale);
  if (_translation.norm() > 0.0)
    libMesh::MeshTools::Modification::translate(
        *_mesh, _translation(0), _translation(1), _translation(2));

  _mesh->prepare_for_use();

  // TriangleManifold's ctor validates Tri3-only + closed + consistently oriented.
  // We only need the validation (mooseError-on-invalid); we do not keep the
  // manifold instance because queryAt() derives the SDF sign from the closest
  // triangle's face normal, which is O(1) and does not need a runtime ray cast.
  TriangleManifold validate(*_mesh, _surface_tolerance);
  (void)validate;

  const auto n_active = _mesh->n_active_elem();
  _centroids.reserve(n_active);
  _triangles.reserve(n_active);
  _vertex_incident_tris.clear();
  _vertex_incident_tris.reserve(3 * n_active);
  for (const auto * elem : _mesh->active_element_ptr_range())
  {
    _centroids.push_back(elem->vertex_average());
    _triangles.push_back(elem);
    for (const auto n : make_range(elem->n_nodes()))
      _vertex_incident_tris[elem->node_id(n)].push_back(elem);
  }

  _kd_tree = std::make_unique<KDTree>(_centroids, /*leaf_max_size=*/10);
}

RealVectorValue
SurfaceMeshContactor::faceNormal(const libMesh::Elem & tri)
{
  const Point & a = tri.point(0);
  const Point & b = tri.point(1);
  const Point & c = tri.point(2);
  RealVectorValue n = (b - a).cross(c - a);
  const Real nn = n.norm();
  if (nn > 0.0)
    n /= nn;
  return n;
}

Point
SurfaceMeshContactor::closestPointOnTriangle(const Point & p, const libMesh::Elem & tri)
{
  // Barycentric closest-point on a triangle: classical seven-region test with
  // clamping to edges/vertices.  Based on Ericson, "Real-Time Collision Detection".
  const Point & a = tri.point(0);
  const Point & b = tri.point(1);
  const Point & c = tri.point(2);

  const Point ab = b - a;
  const Point ac = c - a;
  const Point ap = p - a;

  const Real d1 = ab * ap;
  const Real d2 = ac * ap;
  if (d1 <= 0.0 && d2 <= 0.0)
    return a;

  const Point bp = p - b;
  const Real d3 = ab * bp;
  const Real d4 = ac * bp;
  if (d3 >= 0.0 && d4 <= d3)
    return b;

  const Real vc = d1 * d4 - d3 * d2;
  if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0)
  {
    const Real v = d1 / (d1 - d3);
    return a + v * ab;
  }

  const Point cp = p - c;
  const Real d5 = ab * cp;
  const Real d6 = ac * cp;
  if (d6 >= 0.0 && d5 <= d6)
    return c;

  const Real vb = d5 * d2 - d1 * d6;
  if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0)
  {
    const Real w = d2 / (d2 - d6);
    return a + w * ac;
  }

  const Real va = d3 * d6 - d5 * d4;
  if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0)
  {
    const Real w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
    return b + w * (c - b);
  }

  // Interior of the triangle.
  const Real denom = 1.0 / (va + vb + vc);
  const Real v = vb * denom;
  const Real w = vc * denom;
  return a + ab * v + ac * w;
}

Point
SurfaceMeshContactor::closestSurfacePoint(const Point & x, const libMesh::Elem *& closest_tri) const
{
  std::vector<std::size_t> indices(1);
  _kd_tree->neighborSearch(x, 1, indices);

  const libMesh::Elem * hit = _triangles[indices.front()];
  Point best_cp = closestPointOnTriangle(x, *hit);
  Real best_d2 = (x - best_cp).norm_sq();
  closest_tri = hit;

  // Robustness sweep: also test the hit's edge neighbors.  Handles the case
  // where the true closest triangle is not the one whose centroid is nearest
  // (possible when triangles have very different sizes or when x sits closer
  // to a neighboring triangle's edge than to hit's).  Up to 3 extra
  // closest-point tests for Tri3 -- cheap relative to the KDTree lookup.
  for (const auto s : make_range(hit->n_sides()))
  {
    const libMesh::Elem * neigh = hit->neighbor_ptr(s);
    if (!neigh)
      continue; // safety; a validated closed manifold has no null neighbors.
    const Point cp = closestPointOnTriangle(x, *neigh);
    const Real d2 = (x - cp).norm_sq();
    if (d2 < best_d2)
    {
      best_d2 = d2;
      best_cp = cp;
      closest_tri = neigh;
    }
  }
  return best_cp;
}

LevelSetContactor::Query
SurfaceMeshContactor::queryAtRaw(const Point & x) const
{
  // One KDTree search + neighbor sweep serves gap, normal, and hessian.  The
  // per-quantity accessors below defer to this method, so there is no slow
  // path.
  const libMesh::Elem * tri = nullptr;
  const Point cp = closestSurfacePoint(x, tri);
  const RealVectorValue v = x - cp;
  const Real d = v.norm();

  // Sign of g_LS: derived from the Baerentzen and Aanaes (2005) angle-weighted
  // pseudonormal at the closest surface feature (face interior, edge, or
  // vertex).  For a consistently outward-oriented closed manifold this gives
  // the mathematically correct inside/outside classification on every feature.
  // A single-face-normal heuristic fails at convex corners where the KDTree
  // returns a non-adjacent triangle whose outward direction disagrees with
  // the true local outward -- e.g. a query past the base corner of a pyramid
  // whose closest surface point is the corner vertex, where the closest
  // triangle by centroid can be the top base plane (normal +y) even though
  // the outward direction at the corner has significant lateral components.
  const RealVectorValue n_psn = pseudoNormal(cp, *tri);
  const Real vdotn = v * n_psn;
  const Real sign = vdotn >= 0.0 ? 1.0 : -1.0;

  Query q;
  q.gap = sign * d;

  if (d > _surface_tolerance)
    // grad(g_LS(x)) = sign(x) * (x - CP(x)) / |x - CP(x)|.  Always the outward
    // surface normal at CP: v/d itself on the outside, flipped on the inside.
    q.normal = (sign / d) * v;
  else
    // On-surface: v ~ 0, use the pseudonormal at the closest feature.
    q.normal = n_psn;

  // Piecewise-flat facets -> true Hessian is zero on facet interiors.
  q.hessian = RealTensorValue();
  return q;
}

RealVectorValue
SurfaceMeshContactor::pseudoNormal(const Point & cp, const libMesh::Elem & tri) const
{
  // Classify `cp` against `tri`'s three vertices and three edges.
  // Vertex hit: |cp - v_i| within tolerance.
  // Edge hit: cp within tolerance of the edge segment (both perpendicular
  //   distance to the infinite line and parameter t in [0,1]).
  // Otherwise: interior of `tri`.  A generous tol (the same one used to
  // validate the manifold at load time) keeps the classification robust
  // against the small numerical drift produced by closestPointOnTriangle's
  // barycentric arithmetic.
  const Point & a = tri.point(0);
  const Point & b = tri.point(1);
  const Point & c = tri.point(2);
  const Real tol = _surface_tolerance;

  // Vertex hits.
  int vhit = -1;
  if ((cp - a).norm() < tol)
    vhit = 0;
  else if ((cp - b).norm() < tol)
    vhit = 1;
  else if ((cp - c).norm() < tol)
    vhit = 2;

  if (vhit >= 0)
  {
    // Sum alpha_t * n_face(t) over every triangle t incident to this vertex,
    // where alpha_t is the interior angle at the vertex in t.
    const libMesh::dof_id_type nid = tri.node_id(vhit);
    auto it = _vertex_incident_tris.find(nid);
    if (it == _vertex_incident_tris.end())
      return faceNormal(tri);
    RealVectorValue psn;
    for (const libMesh::Elem * t : it->second)
    {
      unsigned int j = libMesh::invalid_uint;
      for (const auto k : make_range(t->n_nodes()))
        if (t->node_id(k) == nid)
        {
          j = k;
          break;
        }
      if (j == libMesh::invalid_uint)
        continue;
      const Point & A = t->point(j);
      const Point & B = t->point((j + 1) % 3);
      const Point & C = t->point((j + 2) % 3);
      const Point e1 = B - A;
      const Point e2 = C - A;
      const Real l1 = e1.norm();
      const Real l2 = e2.norm();
      if (l1 == 0.0 || l2 == 0.0)
        continue;
      Real cosa = (e1 * e2) / (l1 * l2);
      // clamp acos argument against roundoff.
      if (cosa > 1.0)
        cosa = 1.0;
      else if (cosa < -1.0)
        cosa = -1.0;
      const Real alpha = std::acos(cosa);
      psn += alpha * faceNormal(*t);
    }
    const Real nn = psn.norm();
    if (nn > 0.0)
      return psn / nn;
    return faceNormal(tri);
  }

  // Edge hits.  libMesh's Tri3 side ordering: side s spans nodes {s, (s+1)%3}.
  auto onEdge = [&tol](const Point & p, const Point & p0, const Point & p1)
  {
    const Point e = p1 - p0;
    const Real len2 = e.norm_sq();
    if (len2 == 0.0)
      return false;
    const Real t = ((p - p0) * e) / len2;
    if (t < -tol || t > 1.0 + tol)
      return false;
    const Point proj = p0 + t * e;
    return (p - proj).norm() < tol;
  };
  int ehit = -1;
  if (onEdge(cp, a, b))
    ehit = 0; // side 0: {0, 1}
  else if (onEdge(cp, b, c))
    ehit = 1; // side 1: {1, 2}
  else if (onEdge(cp, c, a))
    ehit = 2; // side 2: {2, 0}

  if (ehit >= 0)
  {
    // Two incident triangles: `tri` and its edge neighbor across side `ehit`.
    // Equal-weight sum of face normals (Baerentzen edge pseudonormal).  A
    // validated closed manifold has a non-null neighbor on every side; fall
    // back to the face normal defensively if we somehow do not.
    const libMesh::Elem * neigh = tri.neighbor_ptr(ehit);
    if (!neigh)
      return faceNormal(tri);
    RealVectorValue psn = faceNormal(tri) + faceNormal(*neigh);
    const Real nn = psn.norm();
    return nn > 0.0 ? psn / nn : faceNormal(tri);
  }

  // Interior of `tri`: the ordinary face normal is exact.
  return faceNormal(tri);
}

Real
SurfaceMeshContactor::signedDistanceRaw(const Point & x) const
{
  return queryAtRaw(x).gap;
}

RealVectorValue
SurfaceMeshContactor::normalRaw(const Point & x) const
{
  return queryAtRaw(x).normal;
}
