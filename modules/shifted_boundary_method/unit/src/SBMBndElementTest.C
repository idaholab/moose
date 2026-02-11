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

using namespace libMesh;

// Public wrapper classes to access protected computeNormal
class SBMBndEdge2ForTest : public SBMBndEdge2
{
public:
  using SBMBndEdge2::SBMBndEdge2;
  Point getNormal() const { return computeNormal(); }
};

class SBMBndTri3ForTest : public SBMBndTri3
{
public:
  using SBMBndTri3::SBMBndTri3;
  Point getNormal() const { return computeNormal(); }
};

TEST(SBMBndElementTest, Edge2Normal)
{
  std::unique_ptr<Edge2> edge(new Edge2());

  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 0.0), 1));

  edge->set_node(0) = n0.get();
  edge->set_node(1) = n1.get();

  SBMBndEdge2ForTest bnd_edge(edge.get());
  Point n = bnd_edge.getNormal();

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

  SBMBndTri3ForTest bnd_tri(tri.get());
  Point n = bnd_tri.getNormal();

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

  SBMBndEdge2ForTest bnd_edge(edge.get());
  Point n = bnd_edge.getNormal();

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

TEST(SBMBndElementTest, Tri3NormalTilted)
{
  std::unique_ptr<Tri3> tri(new Tri3());

  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 1.0), 1));
  std::unique_ptr<Node> n2(new Node(Point(0.0, 1.0, 1.0), 2));

  tri->set_node(0) = n0.get();
  tri->set_node(1) = n1.get();
  tri->set_node(2) = n2.get();

  SBMBndTri3ForTest bnd_tri(tri.get());
  Point n = bnd_tri.getNormal();

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
