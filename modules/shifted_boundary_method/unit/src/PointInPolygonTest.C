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
#include "MooseMesh.h"
#include "libmesh/edge_edge2.h"
#include "PointInPolyhedronCheck.h"

using namespace libMesh;

TEST(PointInPolyhedronCheck, RectanglePointInPolyhedronCheck)
{
  std::vector<std::unique_ptr<SBMBndElementBase>> bd_elements;
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
    e->set_node(0) = n0.get();
    e->set_node(1) = n1.get();
    bd_elements.emplace_back(std::make_unique<SBMBndEdge2>(e.get()));
    nodes.push_back(std::move(n0));
    nodes.push_back(std::move(n1));
    edges.push_back(std::move(e));
  };

  create_edge(p0, p1);
  create_edge(p1, p2);
  create_edge(p2, p3);
  create_edge(p3, p0);

  for (const auto & brute_force : {true, false})
  {
    PointInPolyhedronCheck inout_test(
        bd_elements, std::vector<Point>(), Point(1.0, 0.0, 0.0) /*ray_dir*/, brute_force);

    // Inside
    EXPECT_EQ(inout_test.sideness(Point(0.5, 0.5, 0.0)), SurfaceSide::INSIDE);

    // Outside
    EXPECT_EQ(inout_test.sideness(Point(-0.1, 0.5, 0.0)), SurfaceSide::OUTSIDE);
    EXPECT_EQ(inout_test.sideness(Point(1.5, 1.5, 0.0)), SurfaceSide::OUTSIDE);

    // On edge - result depends on epsilon, should return ON
    EXPECT_EQ(inout_test.sideness(Point(1.0, 0.5, 0.0)), SurfaceSide::ON);
  }
}

TEST(PointInPolyhedronCheck, EpsSensitivityOnEdge)
{
  std::vector<std::unique_ptr<SBMBndElementBase>> bd_elements;
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
    e->set_node(0) = n0.get();
    e->set_node(1) = n1.get();
    bd_elements.emplace_back(std::make_unique<SBMBndEdge2>(e.get()));
    nodes.push_back(std::move(n0));
    nodes.push_back(std::move(n1));
    edges.push_back(std::move(e));
  };
  create_edge(p0, p1);
  create_edge(p1, p2);
  create_edge(p2, p3);
  create_edge(p3, p0);

  Point edge_point(1 + 1e-9, 0.5, 0.0);

  for (const auto & brute_force : {true, false})
  {

    {
      PointInPolyhedronCheck test_libmesh_eps(
          bd_elements, std::vector<Point>(), Point(1.0, 0.0, 0.0), brute_force);
      EXPECT_TRUE(test_libmesh_eps.sideness(edge_point) == SurfaceSide::ON);
    }

    {
      Real small_eps = 1e-15;
      PointInPolyhedronCheck test_small_eps(
          bd_elements, std::vector<Point>(), Point(1.0, 0.0, 0.0), brute_force, small_eps);
      // Expect it is NOT considered ON due to small epsilon
      EXPECT_TRUE(test_small_eps.sideness(edge_point) != SurfaceSide::ON);
    }

    {
      Real large_eps = 1e-3;
      PointInPolyhedronCheck test_large_eps(
          bd_elements, std::vector<Point>(), Point(1.0, 0.0, 0.0), brute_force, large_eps);
      // Expect it IS considered ON due to larger epsilon
      EXPECT_TRUE(test_large_eps.sideness(edge_point) == SurfaceSide::ON);
    }
  }

  Point edge_point2(1 + 1e-5, 0.5, 0.0);

  for (const auto & brute_force : {true, false})
  {

    {
      PointInPolyhedronCheck test_libmesh_eps(
          bd_elements, std::vector<Point>(), Point(1.0, 0.0, 0.0), brute_force);
      EXPECT_TRUE(test_libmesh_eps.sideness(edge_point2) != SurfaceSide::ON);
    }

    {
      Real small_eps = 1e-15;
      PointInPolyhedronCheck test_small_eps(
          bd_elements, std::vector<Point>(), Point(1.0, 0.0, 0.0), brute_force, small_eps);
      // Expect it is NOT considered ON due to small epsilon
      EXPECT_TRUE(test_small_eps.sideness(edge_point2) != SurfaceSide::ON);
    }

    {
      Real large_eps = 1e-2;
      PointInPolyhedronCheck test_large_eps(
          bd_elements, std::vector<Point>(), Point(1.0, 0.0, 0.0), brute_force, large_eps);
      // Expect it IS considered ON due to larger epsilon
      EXPECT_TRUE(test_large_eps.sideness(edge_point2) == SurfaceSide::ON);
    }
  }
}
