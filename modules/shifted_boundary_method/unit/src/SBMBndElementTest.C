//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "SBMBndEdge2.h"
#include "SBMBndTri3.h"
#include "MooseMesh.h"
#include "libmesh/face_tri3.h"
#include "libmesh/edge_edge2.h"
#include "SBMUtils.h"
#include "LineSegment.h"
#include "Ball.h"

using namespace libMesh;

// Minimal SBMBndElementBase subclass that is neither LineSegment nor Triangle.
// Used to drive the unsupported-geometry mooseError branches in
// SBMBndElementBase::intersect and ::computeBoundingBall. The dispatchers
// under test short-circuit before touching the normal, so the supplied
// placeholder normal is never inspected.
class SBMBndUnsupportedForTest : public SBMBndElementBase
{
public:
  using SBMBndElementBase::SBMBndElementBase;
};

TEST(SBMBndElementTest, Edge2Normal)
{
  std::unique_ptr<Edge2> edge(new Edge2());

  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 0.0), 1));

  edge->set_node(0) = n0.get();
  edge->set_node(1) = n1.get();

  SBMBndEdge2 bnd_edge(edge.get());
  Point n = bnd_edge.normal();

  EXPECT_NEAR(n(0), 0.0, 1e-12);
  EXPECT_NEAR(n(2), 0.0, 1e-12);
  EXPECT_NEAR(std::abs(n(1)), 1.0, 1e-12);

  // Line crossing through (0.5, -1) to (0.5, 1)
  Point a(0.5, -1.0, 0.0);
  Point b(0.5, 1.0, 0.0);
  LineSegment line_segment_ab(a, b);
  EXPECT_TRUE(bnd_edge.intersect(line_segment_ab));

  // Line parallel, no intercept
  Point c(0.0, 1.0, 0.0);
  Point d(1.0, 1.0, 0.0);
  LineSegment line_segment_cd(c, d);
  EXPECT_FALSE(bnd_edge.intersect(line_segment_cd));

  // Check distance vector
  Point pt(0.5, 1.0, 0.0);
  Point dist = bnd_edge.distanceFrom(pt);
  EXPECT_NEAR(dist(1), -1.0, 1e-12);
}

TEST(SBMBndElementTest, Tri3Normal)
{
  std::unique_ptr<Tri3> tri(new Tri3());

  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 0.0), 1));
  std::unique_ptr<Node> n2(new Node(Point(0.0, 1.0, 0.0), 2));

  tri->set_node(0) = n0.get();
  tri->set_node(1) = n1.get();
  tri->set_node(2) = n2.get();

  SBMBndTri3 bnd_tri(tri.get());
  Point n = bnd_tri.normal();

  EXPECT_NEAR(n(0), 0.0, 1e-12);
  EXPECT_NEAR(n(1), 0.0, 1e-12);
  EXPECT_NEAR(std::abs(n(2)), 1.0, 1e-12);

  // Line from below passing through triangle center
  Point a(0.3, 0.3, -1.0);
  Point b(0.3, 0.3, 1.0);
  LineSegment line_segment_ab(a, b);
  EXPECT_TRUE(bnd_tri.intersect(line_segment_ab));

  // Line away from triangle
  Point c(2.0, 2.0, -1.0);
  Point d(2.0, 2.0, 1.0);
  LineSegment line_segment_cd(c, d);
  EXPECT_FALSE(bnd_tri.intersect(line_segment_cd));

  // Check distance vector roughly in +Z
  Point pt(0.3, 0.3, 1.0);
  Point dist = bnd_tri.distanceFrom(pt);
  EXPECT_NEAR(dist(2), -1.0, 1e-12);
}

TEST(SBMBndElementTest, Edge2NormalTilted)
{
  std::unique_ptr<Edge2> edge(new Edge2());

  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 1.0, 0.0), 1));

  edge->set_node(0) = n0.get();
  edge->set_node(1) = n1.get();

  SBMBndEdge2 bnd_edge(edge.get());
  Point n = bnd_edge.normal();

  // Expected normal
  const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
  EXPECT_NEAR(n(0), -inv_sqrt2, 1e-12);
  EXPECT_NEAR(n(1), inv_sqrt2, 1e-12);
  EXPECT_NEAR(n(2), 0.0, 1e-12);

  // Line crossing
  Point a(0.5, 0.0, 0.0);
  Point b(0.5, 1.0, 0.0);
  LineSegment line_segment_ab(a, b);
  EXPECT_TRUE(bnd_edge.intersect(line_segment_ab));

  // Line outside
  Point c(1.5, 1.0, 0.0);
  Point d(1.5, 2.0, 0.0);
  LineSegment line_segment_cd(c, d);
  EXPECT_FALSE(bnd_edge.intersect(line_segment_cd));

  // Check distance vector direction
  Point pt(0.5, 0.0, 0.0); // Point below edge
  Point dist = bnd_edge.distanceFrom(pt);
  // Should be mostly pointing along normal
  double dot = dist(0) * n(0) + dist(1) * n(1);
  EXPECT_GT(dot, 0.0); // Same direction
}

TEST(SBMBndElementTest, Edge2DistanceNodeFallback)
{
  // Edge from (0,0,0) to (1,0,0); query point lies far off the line axis so
  // its projection onto the edge falls outside the segment, forcing the side
  // loop in distanceFrom() to take the NODEELEM switch case.
  std::unique_ptr<Edge2> edge(new Edge2());
  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 0.0), 1));
  edge->set_node(0) = n0.get();
  edge->set_node(1) = n1.get();

  SBMBndEdge2 bnd_edge(edge.get());

  // Projection of (2, 1, 0) onto the line is (2, 0, 0) which is outside [0,1].
  // Nearest entity is node (1, 0, 0).
  Point pt(2.0, 1.0, 0.0);
  Point dist = bnd_edge.distanceFrom(pt);
  EXPECT_NEAR(dist(0), -1.0, 1e-12);
  EXPECT_NEAR(dist(1), -1.0, 1e-12);
  EXPECT_NEAR(dist(2), 0.0, 1e-12);
}

TEST(SBMBndElementTest, Tri3NormalTilted)
{
  std::unique_ptr<Tri3> tri(new Tri3());

  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 1.0), 1));
  std::unique_ptr<Node> n2(new Node(Point(0.0, 1.0, 1.0), 2));

  tri->set_node(0) = n0.get();
  tri->set_node(1) = n1.get();
  tri->set_node(2) = n2.get();

  SBMBndTri3 bnd_tri(tri.get());
  Point n = bnd_tri.normal();

  // Expected normal
  const double inv_sqrt3 = 1.0 / std::sqrt(3.0);
  EXPECT_NEAR(n(0), -inv_sqrt3, 1e-12);
  EXPECT_NEAR(n(1), -inv_sqrt3, 1e-12);
  EXPECT_NEAR(n(2), inv_sqrt3, 1e-12);

  // Line from below passing through triangle center
  Point a(0.3, 0.3, -1.0);
  Point b(0.3, 0.3, 2.0);
  LineSegment line_segment_ab(a, b);
  EXPECT_TRUE(bnd_tri.intersect(line_segment_ab));

  // Line away from triangle
  Point c(2.0, 2.0, -1.0);
  Point d(2.0, 2.0, 2.0);
  LineSegment line_segment_cd(c, d);
  EXPECT_FALSE(bnd_tri.intersect(line_segment_cd));

  // Check distance vector
  Point pt(2.0, 0.0, 2.0);
  Point dist = bnd_tri.distanceFrom(pt);
  EXPECT_NEAR(dist(0), -1.0, 1e-12);
  EXPECT_NEAR(dist(1), 0.0, 1e-12);
  EXPECT_NEAR(dist(2), -1.0, 1e-12);

  Point pt2(0.0, 0.0, 0.0);
  Point dist2 = bnd_tri.distanceFrom(pt2);
  EXPECT_NEAR(dist2(0), 0.0, 1e-12);
  EXPECT_NEAR(dist2(1), 0.0, 1e-12);
  EXPECT_NEAR(dist2(2), 0.0, 1e-12);
}

TEST(SBMBndElementTest, ProjectedBoundingBoxDiagonal)
{
  // Tri3 spanning [0,1] x [0,1] in the z=0 plane. Bounding-box diagonal is
  // (1,1,0); projected onto a plane orthogonal to z (i.e. removing the
  // z-component, which is already zero) the tangential diagonal has norm
  // sqrt(2). Projecting onto a plane orthogonal to the diagonal direction
  // itself zeroes out the projection.
  std::unique_ptr<Tri3> tri(new Tri3());
  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 0.0), 1));
  std::unique_ptr<Node> n2(new Node(Point(0.0, 1.0, 0.0), 2));
  tri->set_node(0) = n0.get();
  tri->set_node(1) = n1.get();
  tri->set_node(2) = n2.get();

  SBMBndTri3 bnd_tri(tri.get());

  EXPECT_NEAR(bnd_tri.getProjectedBoundingBoxDiagonal(Point(0.0, 0.0, 1.0) /*normal_dir*/),
              std::sqrt(2.0),
              1e-12);

  const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
  EXPECT_NEAR(
      bnd_tri.getProjectedBoundingBoxDiagonal(Point(inv_sqrt2, inv_sqrt2, 0.0) /*normal_dir*/),
      0.0,
      1e-12);
}

TEST(SBMBndElementTest, BaseDynamicDispatcherIntersectAndBoundingBall)
{
  // Exercise SBMBndElementBase::intersect / computeBoundingBall through a
  // base-class reference; the `using` declarations in the derived classes
  // otherwise route direct calls to LineSegment/Triangle and bypass these
  // dispatchers.
  std::unique_ptr<Edge2> edge(new Edge2());
  std::unique_ptr<Node> e0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> e1(new Node(Point(1.0, 0.0, 0.0), 1));
  edge->set_node(0) = e0.get();
  edge->set_node(1) = e1.get();
  SBMBndEdge2 bnd_edge(edge.get());
  const SBMBndElementBase & edge_base = bnd_edge;

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
  tri->set_node(0) = t0.get();
  tri->set_node(1) = t1.get();
  tri->set_node(2) = t2.get();
  SBMBndTri3 bnd_tri(tri.get());
  const SBMBndElementBase & tri_base = bnd_tri;

  LineSegment piercing(Point(0.3, 0.3, -1.0), Point(0.3, 0.3, 1.0));
  LineSegment missing(Point(2.0, 2.0, -1.0), Point(2.0, 2.0, 1.0));
  EXPECT_TRUE(tri_base.intersect(piercing));
  EXPECT_FALSE(tri_base.intersect(missing));

  const Ball tri_ball = tri_base.computeBoundingBall();
  EXPECT_GT(tri_ball.radius(), 0.0);
}

TEST(SBMBndElementTest, UnsupportedGeometryDispatchersThrow)
{
  // Drive a SBMBndElementBase subclass that is neither LineSegment nor
  // Triangle through the base-class dispatchers; both intersect() and
  // computeBoundingBall() must mooseError on the unsupported geometry type.
  std::unique_ptr<Edge2> edge(new Edge2());
  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 0.0), 1));
  edge->set_node(0) = n0.get();
  edge->set_node(1) = n1.get();

  // Placeholder unit normal; the dispatchers under test never consult it.
  SBMBndUnsupportedForTest bnd(edge.get(), Point(0.0, 0.0, 1.0));
  const SBMBndElementBase & base = bnd;

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
