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
#include "SBMSurfaceDistance.h"
#include "MooseMesh.h"
#include "libmesh/face_tri3.h"
#include "libmesh/edge_edge2.h"

using namespace libMesh;

// The surface-element geometry (normals, intersect, bounding ball, projected
// bounding-box diagonal, unsupported-geometry dispatchers) is covered by the
// framework SurfaceElementTest. These tests cover only the SBM-owned
// SBMUtils::distanceFrom free function.

TEST(SBMSurfaceDistanceTest, Edge2NormalProjection)
{
  std::unique_ptr<Edge2> edge(new Edge2());
  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 0.0), 1));
  edge->set_node(0, n0.get());
  edge->set_node(1, n1.get());

  SurfaceEdge2 surface_edge(edge.get());

  // Point directly above the edge midpoint: distance vector is normal-based.
  Point pt(0.5, 1.0, 0.0);
  Point dist = SBMUtils::distanceFrom(surface_edge, pt);
  EXPECT_NEAR(dist(1), -1.0, 1e-12);
}

TEST(SBMSurfaceDistanceTest, Edge2NodeFallback)
{
  // Edge from (0,0,0) to (1,0,0); query point lies far off the line axis so its
  // projection onto the edge falls outside the segment, forcing the side loop in
  // distanceFrom() to take the NODEELEM/vertex fallback.
  std::unique_ptr<Edge2> edge(new Edge2());
  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 0.0), 1));
  edge->set_node(0, n0.get());
  edge->set_node(1, n1.get());

  SurfaceEdge2 surface_edge(edge.get());

  // Projection of (2, 1, 0) onto the line is (2, 0, 0), outside [0,1]; the
  // nearest entity is node (1, 0, 0).
  Point pt(2.0, 1.0, 0.0);
  Point dist = SBMUtils::distanceFrom(surface_edge, pt);
  EXPECT_NEAR(dist(0), -1.0, 1e-12);
  EXPECT_NEAR(dist(1), -1.0, 1e-12);
  EXPECT_NEAR(dist(2), 0.0, 1e-12);
}

TEST(SBMSurfaceDistanceTest, Edge2TiltedDirection)
{
  std::unique_ptr<Edge2> edge(new Edge2());
  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 1.0, 0.0), 1));
  edge->set_node(0, n0.get());
  edge->set_node(1, n1.get());

  SurfaceEdge2 surface_edge(edge.get());
  const Point n = surface_edge.normal();

  Point pt(0.5, 0.0, 0.0);
  Point dist = SBMUtils::distanceFrom(surface_edge, pt);
  // Distance vector should point along the normal direction.
  const double dot = dist(0) * n(0) + dist(1) * n(1);
  EXPECT_GT(dot, 0.0);
}

TEST(SBMSurfaceDistanceTest, Tri3NormalProjection)
{
  std::unique_ptr<Tri3> tri(new Tri3());
  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 0.0), 1));
  std::unique_ptr<Node> n2(new Node(Point(0.0, 1.0, 0.0), 2));
  tri->set_node(0, n0.get());
  tri->set_node(1, n1.get());
  tri->set_node(2, n2.get());

  SurfaceTri3 surface_tri(tri.get());

  // Point above the triangle interior: distance vector roughly in -Z.
  Point pt(0.3, 0.3, 1.0);
  Point dist = SBMUtils::distanceFrom(surface_tri, pt);
  EXPECT_NEAR(dist(2), -1.0, 1e-12);
}

TEST(SBMSurfaceDistanceTest, Tri3Tilted)
{
  std::unique_ptr<Tri3> tri(new Tri3());
  std::unique_ptr<Node> n0(new Node(Point(0.0, 0.0, 0.0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1.0, 0.0, 1.0), 1));
  std::unique_ptr<Node> n2(new Node(Point(0.0, 1.0, 1.0), 2));
  tri->set_node(0, n0.get());
  tri->set_node(1, n1.get());
  tri->set_node(2, n2.get());

  SurfaceTri3 surface_tri(tri.get());

  // Off-element query: nearest entity is a vertex/edge of the tilted triangle.
  Point pt(2.0, 0.0, 2.0);
  Point dist = SBMUtils::distanceFrom(surface_tri, pt);
  EXPECT_NEAR(dist(0), -1.0, 1e-12);
  EXPECT_NEAR(dist(1), 0.0, 1e-12);
  EXPECT_NEAR(dist(2), -1.0, 1e-12);

  // Query exactly on a vertex: zero distance vector.
  Point pt2(0.0, 0.0, 0.0);
  Point dist2 = SBMUtils::distanceFrom(surface_tri, pt2);
  EXPECT_NEAR(dist2(0), 0.0, 1e-12);
  EXPECT_NEAR(dist2(1), 0.0, 1e-12);
  EXPECT_NEAR(dist2(2), 0.0, 1e-12);
}
