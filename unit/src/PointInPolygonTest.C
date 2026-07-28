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
#include "MooseMesh.h"
#include "libmesh/edge_edge2.h"
#include "AdaptiveRayContainmentCheck.h"

using namespace libMesh;

TEST(AdaptiveRayContainmentCheck, RectangleAdaptiveRayContainmentCheck)
{
  std::vector<std::unique_ptr<SurfaceElement>> bd_elements;
  std::vector<std::unique_ptr<Node>> nodes;
  std::vector<std::unique_ptr<Edge2>> edges;

  Point p0(0.0, 0.0, 0.0);
  Point p1(1.0, 0.0, 0.0);
  Point p2(1.0, 1.0, 0.0);
  Point p3(0.0, 1.0, 0.0);

  dof_id_type node_id = 0;
  auto create_edge = [&](const Point & a, const Point & b)
  {
    auto n0 = std::make_unique<Node>(a, node_id++);
    auto n1 = std::make_unique<Node>(b, node_id++);
    auto e = std::make_unique<Edge2>();
    e->set_node(0, n0.get());
    e->set_node(1, n1.get());
    bd_elements.emplace_back(std::make_unique<SurfaceEdge2>(e.get()));
    nodes.push_back(std::move(n0));
    nodes.push_back(std::move(n1));
    edges.push_back(std::move(e));
  };

  create_edge(p0, p1);
  create_edge(p1, p2);
  create_edge(p2, p3);
  create_edge(p3, p0);

  // USER_SPECIFIED: shoot the ray along +x exactly (no PCA), matching the old
  // explicit-direction constructor this test was written against.
  const RayDirectionOptions ray_opts{RayDirectionMode::USER_SPECIFIED, Point(1.0, 0.0, 0.0)};
  AdaptiveRayContainmentCheck inout_test(bd_elements, std::vector<Point>(), ray_opts);

  // Inside
  EXPECT_EQ(inout_test.sideness(Point(0.5, 0.5, 0.0)), SurfaceSide::INSIDE);

  // Outside
  EXPECT_EQ(inout_test.sideness(Point(-0.1, 0.5, 0.0)), SurfaceSide::OUTSIDE);
  EXPECT_EQ(inout_test.sideness(Point(1.5, 1.5, 0.0)), SurfaceSide::OUTSIDE);

  // On edge - result depends on epsilon, should return ON
  EXPECT_EQ(inout_test.sideness(Point(1.0, 0.5, 0.0)), SurfaceSide::ON);
}

TEST(AdaptiveRayContainmentCheck, PcaFallbackUsesFallbackDirection)
{
  std::vector<std::unique_ptr<SurfaceElement>> bd_elements;
  std::vector<std::unique_ptr<Node>> nodes;
  std::vector<std::unique_ptr<Edge2>> edges;

  // A tall U-shaped polygon makes x the primary (second-variance) PCA direction. The extra
  // collinear vertices at (1.625, 0) and (2, 4) keep the x-y covariance zero. The query is outside
  // in the opening and aligned with (2, 4), so the opposite primary rays have nonzero crossing
  // counts of two and three. The fallback y ray must escape through the opening without crossing
  // the surface.
  const std::array<Point, 10> polygon = {Point(0.0, 0.0, 0.0),
                                         Point(1.625, 0.0, 0.0),
                                         Point(3.0, 0.0, 0.0),
                                         Point(3.0, 6.0, 0.0),
                                         Point(2.0, 6.0, 0.0),
                                         Point(2.0, 4.0, 0.0),
                                         Point(2.0, 2.0, 0.0),
                                         Point(1.0, 2.0, 0.0),
                                         Point(1.0, 6.0, 0.0),
                                         Point(0.0, 6.0, 0.0)};

  dof_id_type node_id = 0;
  for (const auto i : index_range(polygon))
  {
    auto n0 = std::make_unique<Node>(polygon[i], node_id++);
    auto n1 = std::make_unique<Node>(polygon[(i + 1) % polygon.size()], node_id++);
    auto edge = std::make_unique<Edge2>();
    edge->set_node(0, n0.get());
    edge->set_node(1, n1.get());
    bd_elements.emplace_back(std::make_unique<SurfaceEdge2>(edge.get()));
    nodes.push_back(std::move(n0));
    nodes.push_back(std::move(n1));
    edges.push_back(std::move(edge));
  }

  const RayDirectionOptions ray_options{RayDirectionMode::AUTO_PCA, Point()};
  AdaptiveRayContainmentCheck containment(bd_elements, std::vector<Point>(), ray_options);

  EXPECT_EQ(containment.sideness(Point(1.5, 4.0, 0.0)), SurfaceSide::OUTSIDE);
}

namespace
{
/// Build closed-loop EDGE2 boundary elements from an ordered list of polygon vertices, keeping the
/// backing nodes/edges alive in the provided owners. Node ids are assigned per unique vertex and
/// shared between adjacent edges so a vertex the ray passes through is a single, groupable node.
void
buildClosedPolygon(const std::vector<Point> & vertices,
                   std::vector<std::unique_ptr<SurfaceElement>> & bd_elements,
                   std::vector<std::unique_ptr<Node>> & nodes,
                   std::vector<std::unique_ptr<Edge2>> & edges)
{
  const std::size_t n = vertices.size();
  for (const auto i : index_range(vertices))
    nodes.push_back(std::make_unique<Node>(vertices[i], static_cast<dof_id_type>(i)));

  for (const auto i : index_range(vertices))
  {
    auto edge = std::make_unique<Edge2>();
    edge->set_node(0, nodes[i].get());
    edge->set_node(1, nodes[(i + 1) % n].get());
    bd_elements.emplace_back(std::make_unique<SurfaceEdge2>(edge.get()));
    edges.push_back(std::move(edge));
  }
}
}

TEST(AdaptiveRayContainmentCheck, SymmetricLDiagonalVertexHits)
{
  // Symmetric L-shape whose symmetry axis is y = x. The PCA second-variance direction (the 2D ray
  // direction) is exactly (1, 1), so a query point on the diagonal fires a ray straight through the
  // boundary vertices (2, 2) and (1, 1). Before shared-vertex grouping this produced an unstable
  // parity and an eventual "No decision" error; it must now classify cleanly.
  std::vector<std::unique_ptr<SurfaceElement>> bd_elements;
  std::vector<std::unique_ptr<Node>> nodes;
  std::vector<std::unique_ptr<Edge2>> edges;
  buildClosedPolygon({Point(1.0, 1.0, 0.0),
                      Point(3.0, 1.0, 0.0),
                      Point(3.0, 2.0, 0.0),
                      Point(2.0, 2.0, 0.0),
                      Point(2.0, 3.0, 0.0),
                      Point(1.0, 3.0, 0.0)},
                     bd_elements,
                     nodes,
                     edges);

  const RayDirectionOptions ray_options{RayDirectionMode::AUTO_PCA, Point()};
  AdaptiveRayContainmentCheck containment(bd_elements, std::vector<Point>(), ray_options);

  // On the diagonal, inside the bottom arm; its ray grazes the concave vertex (2, 2).
  EXPECT_EQ(containment.sideness(Point(1.875, 1.875, 0.0)), SurfaceSide::INSIDE);
  // On the diagonal, inside the bottom arm; its ray grazes the convex corner (1, 1).
  EXPECT_EQ(containment.sideness(Point(1.25, 1.25, 0.0)), SurfaceSide::INSIDE);
  // On the diagonal, in the removed upper-right square; its ray grazes vertices (2, 2) and (1, 1).
  EXPECT_EQ(containment.sideness(Point(2.5, 2.5, 0.0)), SurfaceSide::OUTSIDE);
}

TEST(AdaptiveRayContainmentCheck, TrueCrossingAtVertex)
{
  // Diamond with left/right vertices at y = 2. A horizontal (+x) ray through the interior passes
  // exactly through a vertex whose two incident edges lie on opposite sides of the ray: a true
  // crossing that must count once (double-counting it would flip the parity to OUTSIDE).
  std::vector<std::unique_ptr<SurfaceElement>> bd_elements;
  std::vector<std::unique_ptr<Node>> nodes;
  std::vector<std::unique_ptr<Edge2>> edges;
  buildClosedPolygon(
      {Point(0.0, 2.0, 0.0), Point(2.0, 0.0, 0.0), Point(4.0, 2.0, 0.0), Point(2.0, 4.0, 0.0)},
      bd_elements,
      nodes,
      edges);

  const RayDirectionOptions ray_options{RayDirectionMode::USER_SPECIFIED, Point(1.0, 0.0, 0.0)};
  AdaptiveRayContainmentCheck containment(bd_elements, std::vector<Point>(), ray_options);

  // Center: the ray to it crosses the left vertex (0, 2) exactly once -> inside.
  EXPECT_EQ(containment.sideness(Point(2.0, 2.0, 0.0)), SurfaceSide::INSIDE);
  // On the same ray line but outside the diamond, past the right vertex (4, 2).
  EXPECT_EQ(containment.sideness(Point(5.0, 2.0, 0.0)), SurfaceSide::OUTSIDE);
}

TEST(AdaptiveRayContainmentCheck, TangentTouchAtSpikeTip)
{
  // Rectangle with a downward spike on the top edge; the spike tip (2, 2) is a local extremum for a
  // horizontal ray, so both incident edges lie above the ray -> a tangential touch that must count
  // zero. (Unconditionally counting a coincident vertex once would wrongly flip the parity here.)
  std::vector<std::unique_ptr<SurfaceElement>> bd_elements;
  std::vector<std::unique_ptr<Node>> nodes;
  std::vector<std::unique_ptr<Edge2>> edges;
  buildClosedPolygon({Point(0.0, 0.0, 0.0),
                      Point(4.0, 0.0, 0.0),
                      Point(4.0, 4.0, 0.0),
                      Point(2.5, 4.0, 0.0),
                      Point(2.0, 2.0, 0.0),
                      Point(1.5, 4.0, 0.0),
                      Point(0.0, 4.0, 0.0)},
                     bd_elements,
                     nodes,
                     edges);

  const RayDirectionOptions ray_options{RayDirectionMode::USER_SPECIFIED, Point(1.0, 0.0, 0.0)};
  AdaptiveRayContainmentCheck containment(bd_elements, std::vector<Point>(), ray_options);

  // At y = 2 the ray to this interior point grazes the spike tip (2, 2); the touch must not be
  // counted as a crossing, so the point stays inside.
  EXPECT_EQ(containment.sideness(Point(3.5, 2.0, 0.0)), SurfaceSide::INSIDE);
}

TEST(AdaptiveRayContainmentCheck, NearMissVertex)
{
  // Diamond with left/right vertices at y = 2. A horizontal (+x) ray offset from y = 2 by a small
  // amount passes just beside the side vertices instead of through them, so each side is a clean
  // interior crossing with no vertex/collinear special case. This guards the tolerance bands: the
  // 1e-6 offset must stay well outside the endpoint band (edge_param_tol = 1e-8) and be read as an
  // interior hit, not misgrouped as a vertex hit.
  std::vector<std::unique_ptr<SurfaceElement>> bd_elements;
  std::vector<std::unique_ptr<Node>> nodes;
  std::vector<std::unique_ptr<Edge2>> edges;
  buildClosedPolygon(
      {Point(0.0, 2.0, 0.0), Point(2.0, 0.0, 0.0), Point(4.0, 2.0, 0.0), Point(2.0, 4.0, 0.0)},
      bd_elements,
      nodes,
      edges);

  const RayDirectionOptions ray_options{RayDirectionMode::USER_SPECIFIED, Point(1.0, 0.0, 0.0)};
  AdaptiveRayContainmentCheck containment(bd_elements, std::vector<Point>(), ray_options);

  // Interior points whose ray passes just above / just below the side vertices (near miss).
  EXPECT_EQ(containment.sideness(Point(2.0, 2.0 + 1.0e-6, 0.0)), SurfaceSide::INSIDE);
  EXPECT_EQ(containment.sideness(Point(2.0, 2.0 - 1.0e-6, 0.0)), SurfaceSide::INSIDE);
  // On a near-miss line but outside the diamond, past the right vertex.
  EXPECT_EQ(containment.sideness(Point(5.0, 2.0 + 1.0e-6, 0.0)), SurfaceSide::OUTSIDE);
}

TEST(AdaptiveRayContainmentCheck, CollinearRayAlongEdge)
{
  // Hexagon = [0,4]x[0,4] with a [4,6]x[2,4] bump, so the edge (4,2)-(6,2) lies on the line y = 2.
  // A +x ray from the interior point (2, 2) runs exactly along that whole edge. The boundary is
  // below the ray before the edge and above it after, so the collinear feature is a true crossing:
  // (2, 2) must classify INSIDE.
  std::vector<std::unique_ptr<SurfaceElement>> bd_elements;
  std::vector<std::unique_ptr<Node>> nodes;
  std::vector<std::unique_ptr<Edge2>> edges;
  buildClosedPolygon({Point(0.0, 0.0, 0.0),
                      Point(4.0, 0.0, 0.0),
                      Point(4.0, 2.0, 0.0),
                      Point(6.0, 2.0, 0.0),
                      Point(6.0, 4.0, 0.0),
                      Point(0.0, 4.0, 0.0)},
                     bd_elements,
                     nodes,
                     edges);

  const RayDirectionOptions ray_options{RayDirectionMode::USER_SPECIFIED, Point(1.0, 0.0, 0.0)};
  AdaptiveRayContainmentCheck containment(bd_elements, std::vector<Point>(), ray_options);

  EXPECT_EQ(containment.sideness(Point(2.0, 2.0, 0.0)), SurfaceSide::INSIDE);
}

TEST(AdaptiveRayContainmentCheck, EpsSensitivityOnEdge)
{
  std::vector<std::unique_ptr<SurfaceElement>> bd_elements;
  std::vector<std::unique_ptr<Node>> nodes;
  std::vector<std::unique_ptr<Edge2>> edges;

  Point p0(0.0, 0.0, 0.0);
  Point p1(1.0, 0.0, 0.0);
  Point p2(1.0, 1.0, 0.0);
  Point p3(0.0, 1.0, 0.0);
  dof_id_type node_id = 0;
  auto create_edge = [&](const Point & a, const Point & b)
  {
    auto n0 = std::make_unique<Node>(a, node_id++);
    auto n1 = std::make_unique<Node>(b, node_id++);
    auto e = std::make_unique<Edge2>();
    e->set_node(0, n0.get());
    e->set_node(1, n1.get());
    bd_elements.emplace_back(std::make_unique<SurfaceEdge2>(e.get()));
    nodes.push_back(std::move(n0));
    nodes.push_back(std::move(n1));
    edges.push_back(std::move(e));
  };
  create_edge(p0, p1);
  create_edge(p1, p2);
  create_edge(p2, p3);
  create_edge(p3, p0);

  // USER_SPECIFIED: shoot the ray along +x exactly (no PCA), matching the old
  // explicit-direction constructor this test was written against.
  const RayDirectionOptions ray_opts{RayDirectionMode::USER_SPECIFIED, Point(1.0, 0.0, 0.0)};

  Point edge_point(1 + 1e-9, 0.5, 0.0);

  {
    AdaptiveRayContainmentCheck test_libmesh_eps(bd_elements, std::vector<Point>(), ray_opts);
    EXPECT_TRUE(test_libmesh_eps.sideness(edge_point) == SurfaceSide::ON);
  }

  {
    Real small_eps = 1e-15;
    AdaptiveRayContainmentCheck test_small_eps(
        bd_elements, std::vector<Point>(), ray_opts, small_eps);
    // Expect it is NOT considered ON due to small epsilon
    EXPECT_TRUE(test_small_eps.sideness(edge_point) != SurfaceSide::ON);
  }

  {
    Real large_eps = 1e-3;
    AdaptiveRayContainmentCheck test_large_eps(
        bd_elements, std::vector<Point>(), ray_opts, large_eps);
    // Expect it IS considered ON due to larger epsilon
    EXPECT_TRUE(test_large_eps.sideness(edge_point) == SurfaceSide::ON);
  }

  Point edge_point2(1 + 1e-5, 0.5, 0.0);

  {
    AdaptiveRayContainmentCheck test_libmesh_eps(bd_elements, std::vector<Point>(), ray_opts);
    EXPECT_TRUE(test_libmesh_eps.sideness(edge_point2) != SurfaceSide::ON);
  }

  {
    Real small_eps = 1e-15;
    AdaptiveRayContainmentCheck test_small_eps(
        bd_elements, std::vector<Point>(), ray_opts, small_eps);
    // Expect it is NOT considered ON due to small epsilon
    EXPECT_TRUE(test_small_eps.sideness(edge_point2) != SurfaceSide::ON);
  }

  {
    Real large_eps = 1e-2;
    AdaptiveRayContainmentCheck test_large_eps(
        bd_elements, std::vector<Point>(), ray_opts, large_eps);
    // Expect it IS considered ON due to larger epsilon
    EXPECT_TRUE(test_large_eps.sideness(edge_point2) == SurfaceSide::ON);
  }
}

TEST(AdaptiveRayContainmentCheck, EmptyBoundaryElementsThrow)
{
  // An empty boundary set must be rejected with a runtime error rather than
  // dereferencing _bd_elements[0], which is undefined behavior in opt builds
  // where the constructor's internal check would otherwise be compiled out.
  std::vector<std::unique_ptr<SurfaceElement>> empty_bd_elements;
  const RayDirectionOptions ray_opts{RayDirectionMode::USER_SPECIFIED, Point(1.0, 0.0, 0.0)};
  EXPECT_THROW(
      {
        try
        {
          AdaptiveRayContainmentCheck check(empty_bd_elements, std::vector<Point>(), ray_opts);
        }
        catch (const std::exception & e)
        {
          EXPECT_NE(std::string(e.what()).find("must not be empty"), std::string::npos);
          throw;
        }
      },
      std::exception);
}
