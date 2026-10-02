//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LevelSetContactor.h"

#include "libmesh/id_types.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace libMesh
{
class ReplicatedMesh;
class Elem;
}

class KDTree;
class TriangleManifold;

/**
 * Rigid contactor described by a user-supplied closed, oriented, triangulated
 * surface mesh (typically STL).  Provides the same LevelSetContactor
 * `signedDistance` / `normal` interface as SphereContactor, computed as:
 *
 *   sign(x)   = -1 if x is inside the manifold, +1 outside
 *               (from TriangleManifold::contains)
 *   |g_LS(x)| = distance from x to the closest point on any triangle
 *               (KDTree nearest-centroid search + closest-point-on-triangle)
 *   n(x)      = grad(g_LS) = (x - closest_pt) / |x - closest_pt|
 *
 * The Hessian is inherited zero from LevelSetContactor: piecewise-flat facets
 * have zero true Hessian in facet interiors, so the finite-strain geometric
 * Jacobian term is dropped.
 */
class SurfaceMeshContactor : public LevelSetContactor
{
public:
  static InputParameters validParams();
  SurfaceMeshContactor(const InputParameters & parameters);
  virtual ~SurfaceMeshContactor();

  virtual void initialSetup() override;

protected:
  virtual Real signedDistanceRaw(const Point & x) const override;
  virtual RealVectorValue normalRaw(const Point & x) const override;
  virtual Query queryAtRaw(const Point & x) const override;

private:
  /**
   * Find the closest point on `tri` (assumed Tri3) to `p`.  Standard
   * barycentric projection with clamping to edges/vertices.
   */
  static Point closestPointOnTriangle(const Point & p, const libMesh::Elem & tri);

  /**
   * Outward unit normal of a Tri3 element (assumes consistent CCW winding
   * relative to the outward direction; validated at initialSetup).
   */
  static RealVectorValue faceNormal(const libMesh::Elem & tri);

  /**
   * KDTree lookup for the nearest triangle centroid, then also test that
   * triangle's edge neighbors (up to 3 for Tri3).  Robust to moderate
   * variation in triangle sizes; a KDTree hit whose true nearest surface
   * point actually lies on a neighbor is recovered by the neighbor sweep.
   * Populates `closest_tri` with the triangle carrying the closest point.
   */
  Point closestSurfacePoint(const Point & x, const libMesh::Elem *& closest_tri) const;

  /**
   * Angle-weighted pseudonormal at `cp` on `tri`, following Baerentzen and
   * Aanaes (2005).  For a `cp` in the interior of `tri` this is just the
   * face normal.  For a `cp` on an edge shared by two triangles it is the
   * (unit-weight) sum of the two face normals; for a `cp` at a vertex it
   * is the interior-angle-weighted sum over every triangle incident to
   * that vertex.  Using it (instead of the closest triangle's face normal
   * alone) gives the mathematically correct SDF sign at edges and vertices
   * of the manifold -- the face-normal-only heuristic can pick the wrong
   * sign at convex corners when the KDTree returns a non-adjacent triangle
   * whose outward direction differs from the true local outward at the
   * shared feature.
   */
  RealVectorValue pseudoNormal(const Point & cp, const libMesh::Elem & tri) const;

  const FileName _file;
  const Point _translation;
  const Real _scale;
  const Real _surface_tolerance;

  std::unique_ptr<libMesh::ReplicatedMesh> _mesh;
  std::unique_ptr<KDTree> _kd_tree;
  std::vector<Point> _centroids;
  std::vector<const libMesh::Elem *> _triangles;

  /// node_id -> every triangle in the surface mesh that includes that node.
  /// Populated in initialSetup once the mesh is de-duplicated and prepared;
  /// used by pseudoNormal() when the closest surface point is at a vertex.
  std::unordered_map<libMesh::dof_id_type, std::vector<const libMesh::Elem *>>
      _vertex_incident_tris;
};
