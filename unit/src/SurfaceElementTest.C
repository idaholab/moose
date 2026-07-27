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

// Minimal SurfaceElement subclass that is neither LineSegment nor Triangle.
// Used to drive the unsupported-geometry mooseError branches in
// SurfaceElement::intersect and ::computeBoundingBall. The dispatchers
// under test short-circuit before touching the normal, so the supplied
// placeholder normal is never inspected.
class SurfaceElementUnsupportedForTest : public SurfaceElement
{
public:
  using SurfaceElement::SurfaceElement;
};

TEST(SurfaceElementTest, Edge2Normal)
{
  std::unique_ptr<Edge2> edge(new Edge2());

  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 0.0), 1));

  edge->set_node(0, n0.get());
  edge->set_node(1, n1.get());

  SurfaceEdge2 surface_edge(edge.get());
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
  std::unique_ptr<Tri3> tri(new Tri3());

  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 0.0), 1));
  std::unique_ptr<Node> n2(new Node(Point(0.0, 1.0, 0.0), 2));

  tri->set_node(0, n0.get());
  tri->set_node(1, n1.get());
  tri->set_node(2, n2.get());

  SurfaceTri3 surface_tri(tri.get());
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
  std::unique_ptr<Edge2> edge(new Edge2());

  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 1.0, 0.0), 1));

  edge->set_node(0, n0.get());
  edge->set_node(1, n1.get());

  SurfaceEdge2 surface_edge(edge.get());
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
  std::unique_ptr<Tri3> tri(new Tri3());

  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 1.0), 1));
  std::unique_ptr<Node> n2(new Node(Point(0.0, 1.0, 1.0), 2));

  tri->set_node(0, n0.get());
  tri->set_node(1, n1.get());
  tri->set_node(2, n2.get());

  SurfaceTri3 surface_tri(tri.get());
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
  std::unique_ptr<Tri3> tri(new Tri3());
  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 0.0), 1));
  std::unique_ptr<Node> n2(new Node(Point(0.0, 1.0, 0.0), 2));
  tri->set_node(0, n0.get());
  tri->set_node(1, n1.get());
  tri->set_node(2, n2.get());

  SurfaceTri3 surface_tri(tri.get());

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
  std::unique_ptr<Edge2> edge(new Edge2());
  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 1.0, 0.0), 1));
  edge->set_node(0, n0.get());
  edge->set_node(1, n1.get());
  SurfaceEdge2 surface_edge(edge.get());

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
  std::unique_ptr<Tri3> tri(new Tri3());
  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 1.0, 0.0), 1));
  std::unique_ptr<Node> n2(new Node(Point(0.0, 0.0, 1.0), 2));
  tri->set_node(0, n0.get());
  tri->set_node(1, n1.get());
  tri->set_node(2, n2.get());
  SurfaceTri3 surface_tri(tri.get());

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
  // base-class reference; the `using` declarations in the derived classes
  // otherwise route direct calls to LineSegment/Triangle and bypass these
  // dispatchers.
  std::unique_ptr<Edge2> edge(new Edge2());
  std::unique_ptr<Node> e0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> e1(new Node(Point(1.0, 0.0, 0.0), 1));
  edge->set_node(0, e0.get());
  edge->set_node(1, e1.get());
  SurfaceEdge2 surface_edge(edge.get());
  const SurfaceElement & edge_base = surface_edge;

  LineSegment crossing(Point(0.5, -1.0, 0.0), Point(0.5, 1.0, 0.0));
  LineSegment parallel(Point(0.0, 1.0, 0.0), Point(1.0, 1.0, 0.0));
  EXPECT_TRUE(edge_base.intersect(crossing));
  EXPECT_FALSE(edge_base.intersect(parallel));

  const Ball edge_ball = edge_base.computeBoundingBall();
  EXPECT_NEAR(edge_ball.center()(0), 0.5, 1e-12);
  EXPECT_NEAR(edge_ball.radius(), 0.5, 1e-12);

  std::unique_ptr<Tri3> tri(new Tri3());
  std::unique_ptr<Node> t0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> t1(new Node(Point(1.0, 0.0, 0.0), 1));
  std::unique_ptr<Node> t2(new Node(Point(0.0, 1.0, 0.0), 2));
  tri->set_node(0, t0.get());
  tri->set_node(1, t1.get());
  tri->set_node(2, t2.get());
  SurfaceTri3 surface_tri(tri.get());
  const SurfaceElement & tri_base = surface_tri;

  LineSegment piercing(Point(0.3, 0.3, -1.0), Point(0.3, 0.3, 1.0));
  LineSegment missing(Point(2.0, 2.0, -1.0), Point(2.0, 2.0, 1.0));
  EXPECT_TRUE(tri_base.intersect(piercing));
  EXPECT_FALSE(tri_base.intersect(missing));

  const Ball tri_ball = tri_base.computeBoundingBall();
  EXPECT_GT(tri_ball.radius(), 0.0);
}

TEST(SurfaceElementTest, UnsupportedGeometryDispatchersThrow)
{
  // Drive a SurfaceElement subclass that is neither LineSegment nor Triangle
  // through the base-class dispatchers; both intersect() and
  // computeBoundingBall() must mooseError on the unsupported geometry type.
  std::unique_ptr<Edge2> edge(new Edge2());
  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 0.0), 1));
  edge->set_node(0, n0.get());
  edge->set_node(1, n1.get());

  // Placeholder unit normal; the dispatchers under test never consult it.
  SurfaceElementUnsupportedForTest surf(edge.get(), Point(0.0, 0.0, 1.0));
  const SurfaceElement & base = surf;

  LineSegment line(Point(0.5, -1.0, 0.0), Point(0.5, 1.0, 0.0));
  EXPECT_THROW(
      {
        try
        {
          base.intersect(line);
        }
        catch (const std::exception & e)
        {
          EXPECT_NE(std::string(e.what()).find("unsupported geometry type"), std::string::npos);
          throw;
        }
      },
      std::exception);

  EXPECT_THROW(
      {
        try
        {
          base.computeBoundingBall();
        }
        catch (const std::exception & e)
        {
          EXPECT_NE(std::string(e.what()).find("unsupported geometry type"), std::string::npos);
          throw;
        }
      },
      std::exception);
}
