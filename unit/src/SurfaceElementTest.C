//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "SurfaceEdge2.h"
#include "SurfaceTri3.h"
#include "MooseMesh.h"
#include "libmesh/face_tri3.h"
#include "libmesh/edge_edge2.h"
#include "LineSegment.h"
#include "Ball.h"

using namespace libMesh;

namespace
{
// Owns an Edge2 and its two nodes; a SurfaceEdge2 wrapping owner.edge is valid only while the
// returned owner is alive.
struct Edge2Owner
{
  std::unique_ptr<Node> n0, n1;
  std::unique_ptr<Edge2> edge;
};

Edge2Owner
makeEdge2(const Point & a, const Point & b)
{
  Edge2Owner o{
      std::make_unique<Node>(a, 0), std::make_unique<Node>(b, 1), std::make_unique<Edge2>()};
  o.edge->set_node(0, o.n0.get());
  o.edge->set_node(1, o.n1.get());
  return o;
}

// Owns a Tri3 and its three nodes; see Edge2Owner for the lifetime contract.
struct Tri3Owner
{
  std::unique_ptr<Node> n0, n1, n2;
  std::unique_ptr<Tri3> tri;
};

Tri3Owner
makeTri3(const Point & a, const Point & b, const Point & c)
{
  Tri3Owner o{std::make_unique<Node>(a, 0),
              std::make_unique<Node>(b, 1),
              std::make_unique<Node>(c, 2),
              std::make_unique<Tri3>()};
  o.tri->set_node(0, o.n0.get());
  o.tri->set_node(1, o.n1.get());
  o.tri->set_node(2, o.n2.get());
  return o;
}
}

TEST(SurfaceElementTest, Edge2Normal)
{
  const auto owner = makeEdge2(Point(0.0, 0.0, 0.0), Point(1.0, 0.0, 0.0));
  SurfaceEdge2 surface_edge(owner.edge.get());
  Point n = surface_edge.normal();

  EXPECT_NEAR(n(0), 0.0, 1e-12);
  EXPECT_NEAR(n(2), 0.0, 1e-12);
  EXPECT_NEAR(std::abs(n(1)), 1.0, 1e-12);

  // Line crossing through (0.5, -1) to (0.5, 1)
  Point a(0.5, -1.0, 0.0);
  Point b(0.5, 1.0, 0.0);
  LineSegment line_segment_ab(a, b);
  EXPECT_TRUE(surface_edge.intersect(line_segment_ab));

  // Line parallel, no intercept
  Point c(0.0, 1.0, 0.0);
  Point d(1.0, 1.0, 0.0);
  LineSegment line_segment_cd(c, d);
  EXPECT_FALSE(surface_edge.intersect(line_segment_cd));
}

TEST(SurfaceElementTest, Tri3Normal)
{
  const auto owner = makeTri3(Point(0.0, 0.0, 0.0), Point(1.0, 0.0, 0.0), Point(0.0, 1.0, 0.0));
  SurfaceTri3 surface_tri(owner.tri.get());
  Point n = surface_tri.normal();

  EXPECT_NEAR(n(0), 0.0, 1e-12);
  EXPECT_NEAR(n(1), 0.0, 1e-12);
  EXPECT_NEAR(std::abs(n(2)), 1.0, 1e-12);

  // Line from below passing through triangle center
  Point a(0.3, 0.3, -1.0);
  Point b(0.3, 0.3, 1.0);
  LineSegment line_segment_ab(a, b);
  EXPECT_TRUE(surface_tri.intersect(line_segment_ab));

  // Line away from triangle
  Point c(2.0, 2.0, -1.0);
  Point d(2.0, 2.0, 1.0);
  LineSegment line_segment_cd(c, d);
  EXPECT_FALSE(surface_tri.intersect(line_segment_cd));
}

TEST(SurfaceElementTest, Edge2NormalTilted)
{
  const auto owner = makeEdge2(Point(0.0, 0.0, 0.0), Point(1.0, 1.0, 0.0));
  SurfaceEdge2 surface_edge(owner.edge.get());
  Point n = surface_edge.normal();

  // Expected normal
  const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
  EXPECT_NEAR(n(0), -inv_sqrt2, 1e-12);
  EXPECT_NEAR(n(1), inv_sqrt2, 1e-12);
  EXPECT_NEAR(n(2), 0.0, 1e-12);

  // Line crossing
  Point a(0.5, 0.0, 0.0);
  Point b(0.5, 1.0, 0.0);
  LineSegment line_segment_ab(a, b);
  EXPECT_TRUE(surface_edge.intersect(line_segment_ab));

  // Line outside
  Point c(1.5, 1.0, 0.0);
  Point d(1.5, 2.0, 0.0);
  LineSegment line_segment_cd(c, d);
  EXPECT_FALSE(surface_edge.intersect(line_segment_cd));
}

TEST(SurfaceElementTest, Tri3NormalTilted)
{
  const auto owner = makeTri3(Point(0.0, 0.0, 0.0), Point(1.0, 0.0, 1.0), Point(0.0, 1.0, 1.0));
  SurfaceTri3 surface_tri(owner.tri.get());
  Point n = surface_tri.normal();

  // Expected normal
  const double inv_sqrt3 = 1.0 / std::sqrt(3.0);
  EXPECT_NEAR(n(0), -inv_sqrt3, 1e-12);
  EXPECT_NEAR(n(1), -inv_sqrt3, 1e-12);
  EXPECT_NEAR(n(2), inv_sqrt3, 1e-12);

  // Line from below passing through triangle center
  Point a(0.3, 0.3, -1.0);
  Point b(0.3, 0.3, 2.0);
  LineSegment line_segment_ab(a, b);
  EXPECT_TRUE(surface_tri.intersect(line_segment_ab));

  // Line away from triangle
  Point c(2.0, 2.0, -1.0);
  Point d(2.0, 2.0, 2.0);
  LineSegment line_segment_cd(c, d);
  EXPECT_FALSE(surface_tri.intersect(line_segment_cd));
}

TEST(SurfaceElementTest, ProjectedBoundingBoxDiagonal)
{
  // Tri3 spanning [0,1] x [0,1] in the z=0 plane. The result is the diameter of
  // the AABB shadow on the plane orthogonal to normal_dir, i.e. the longest of the
  // four projected space diagonals. For normal_dir = z, the (1,1,0) diagonal stays
  // in-plane with norm sqrt(2). For normal_dir along the (1,1,0) diagonal, that
  // diagonal projects to zero, but the (1,-1,0) diagonal is orthogonal to normal_dir
  // and still projects to sqrt(2); the footprint diameter is therefore sqrt(2), not
  // zero (projecting only the main diagonal would wrongly report zero here).
  const auto owner = makeTri3(Point(0.0, 0.0, 0.0), Point(1.0, 0.0, 0.0), Point(0.0, 1.0, 0.0));
  SurfaceTri3 surface_tri(owner.tri.get());

  EXPECT_NEAR(surface_tri.getProjectedBoundingBoxDiagonal(Point(0.0, 0.0, 1.0) /*normal_dir*/),
              std::sqrt(2.0),
              1e-12);

  const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
  EXPECT_NEAR(
      surface_tri.getProjectedBoundingBoxDiagonal(Point(inv_sqrt2, inv_sqrt2, 0.0) /*normal_dir*/),
      std::sqrt(2.0),
      1e-12);
}

TEST(SurfaceElementTest, ProjectedBoundingBoxDiagonalAlignedEdge)
{
  // Edge2 whose AABB main diagonal (max - min) is (1,1,0). Shooting the ray exactly
  // along that diagonal makes the main-diagonal projection vanish, but the orthogonal
  // (1,-1,0) space diagonal still has tangential norm sqrt(2). The result must be
  // sqrt(2); reporting only the single main diagonal would underestimate it to 0 and
  // shrink the KD-tree search radius enough to miss real ray/element crossings.
  const auto owner = makeEdge2(Point(0.0, 0.0, 0.0), Point(1.0, 1.0, 0.0));
  SurfaceEdge2 surface_edge(owner.edge.get());

  const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
  EXPECT_NEAR(
      surface_edge.getProjectedBoundingBoxDiagonal(Point(inv_sqrt2, inv_sqrt2, 0.0) /*normal_dir*/),
      std::sqrt(2.0),
      1e-12);
}

TEST(SurfaceElementTest, ProjectedBoundingBoxDiagonalTilted3D)
{
  // Tri3 with AABB [0,1]^3 (main diagonal (1,1,1)). Shooting along the main diagonal
  // zeroes its projection, but the other space diagonals, e.g. (1,1,-1), keep a large
  // tangential component, so the exhaustive four-diagonal maximum is nonzero. This
  // exercises the 3D case the single-main-diagonal formula would underestimate.
  const auto owner = makeTri3(Point(0.0, 0.0, 0.0), Point(1.0, 1.0, 0.0), Point(0.0, 0.0, 1.0));
  SurfaceTri3 surface_tri(owner.tri.get());

  const Point main_diag(1.0, 1.0, 1.0);
  const Point normal_dir = main_diag / main_diag.norm();

  // Expected: max over the four space diagonals (1, +/-1, +/-1) of the tangential norm.
  // The main diagonal (1,1,1) projects to 0; e.g. (1,1,-1) projects to a length of
  // sqrt(8/3). That is the diameter of the projected AABB shadow.
  const Real expected = std::sqrt(8.0 / 3.0);
  EXPECT_NEAR(surface_tri.getProjectedBoundingBoxDiagonal(normal_dir), expected, 1e-12);
}

TEST(SurfaceElementTest, BaseDynamicDispatcherIntersectAndBoundingBall)
{
  // Exercise SurfaceElement::intersect / computeBoundingBall through a
  // base-class reference, so that the derived overrides are reached by virtual
  // dispatch rather than by the direct-call path a concrete-type reference takes.
  const auto edge_owner = makeEdge2(Point(0.0, 0.0, 0.0), Point(1.0, 0.0, 0.0));
  SurfaceEdge2 surface_edge(edge_owner.edge.get());
  const SurfaceElement & edge_base = surface_edge;

  LineSegment crossing(Point(0.5, -1.0, 0.0), Point(0.5, 1.0, 0.0));
  LineSegment parallel(Point(0.0, 1.0, 0.0), Point(1.0, 1.0, 0.0));
  EXPECT_TRUE(edge_base.intersect(crossing));
  EXPECT_FALSE(edge_base.intersect(parallel));

  const Ball edge_ball = edge_base.computeBoundingBall();
  EXPECT_NEAR(edge_ball.center()(0), 0.5, 1e-12);
  EXPECT_NEAR(edge_ball.radius(), 0.5, 1e-12);

  const auto tri_owner = makeTri3(Point(0.0, 0.0, 0.0), Point(1.0, 0.0, 0.0), Point(0.0, 1.0, 0.0));
  SurfaceTri3 surface_tri(tri_owner.tri.get());
  const SurfaceElement & tri_base = surface_tri;

  LineSegment piercing(Point(0.3, 0.3, -1.0), Point(0.3, 0.3, 1.0));
  LineSegment missing(Point(2.0, 2.0, -1.0), Point(2.0, 2.0, 1.0));
  EXPECT_TRUE(tri_base.intersect(piercing));
  EXPECT_FALSE(tri_base.intersect(missing));

  const Ball tri_ball = tri_base.computeBoundingBall();
  // Centroid-centered ball: center = (p0 + p1 + p2) / 3, radius = max vertex distance. For the
  // right triangle (0,0,0)/(1,0,0)/(0,1,0) that is center (1/3, 1/3, 0) and radius sqrt(5)/3.
  EXPECT_NEAR(tri_ball.center()(0), 1.0 / 3.0, 1e-12);
  EXPECT_NEAR(tri_ball.center()(1), 1.0 / 3.0, 1e-12);
  EXPECT_NEAR(tri_ball.center()(2), 0.0, 1e-12);
  EXPECT_NEAR(tri_ball.radius(), std::sqrt(5.0) / 3.0, 1e-12);
}
