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
