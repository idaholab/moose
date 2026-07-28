//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "ArbitraryQuadrature.h"
#include "MooseLagrangeHelpers.h"
#include "MortarSegmentHelper.h"
#include "MortarUtils.h"

#include "libmesh/elem.h"
#include "libmesh/replicated_mesh.h"

#include <array>
#include <memory>
#include <vector>

using namespace libMesh;

namespace
{
Point
mapPoint(const ElemType type, const std::vector<Point> & nodes, const Point & reference_point)
{
  Point point;
  for (const auto node : index_range(nodes))
    point += Moose::fe_lagrange_2D_shape(type, FIRST, node, reference_point) * nodes[node];
  return point;
}

Point
projectPoint(const ElemType parent_type,
             const std::vector<Point> & parent_points,
             const Point & physical_target,
             const Point & projection_normal,
             const Real clipping_area_tolerance = 0,
             const bool compatibility_wrapper = false)
{
  Parallel::Communicator communicator;
  ReplicatedMesh mesh(communicator, 2);
  mesh.set_spatial_dimension(3);
  const auto subpatch_index = mesh.add_elem_integer("subpatch");

  auto parent = Elem::build(parent_type);
  parent->set_id(0);
  for (const auto node : parent->node_index_range())
  {
    auto * const mesh_node = mesh.add_point(parent_points[node]);
    parent->set_node(node, mesh_node);
  }
  const Elem * const parent_ptr = mesh.add_elem(std::move(parent));

  Real parent_scale = 0;
  for (const auto first : index_range(parent_points))
    for (const auto second : make_range(first + 1, parent_points.size()))
      parent_scale = std::max(parent_scale, (parent_points[second] - parent_points[first]).norm());
  const Real segment_scale = parent_scale * 1e-4;

  auto segment = Elem::build(TRI3);
  segment->set_id(1);
  const std::array<Point, 3> segment_points = {physical_target + segment_scale * Point(1, 0, 0),
                                               physical_target + segment_scale * Point(0, 1, 0),
                                               physical_target - segment_scale * Point(1, 1, 0)};
  for (const auto node : segment->node_index_range())
  {
    auto * const mesh_node = mesh.add_point(segment_points[node]);
    segment->set_node(node, mesh_node);
  }
  Elem * const segment_ptr = mesh.add_elem(std::move(segment));
  segment_ptr->set_extra_integer(subpatch_index, 0);

  ArbitraryQuadrature quadrature(2);
  quadrature.setPoints({Point(1.0 / 3.0, 1.0 / 3.0)});
  std::vector<Point> mapped_points;
  if (compatibility_wrapper)
    Moose::Mortar::projectQPoints3d(
        segment_ptr, parent_ptr, subpatch_index, quadrature, mapped_points);
  else
    Moose::Mortar::projectQPoints3d(segment_ptr,
                                    parent_ptr,
                                    subpatch_index,
                                    projection_normal,
                                    clipping_area_tolerance,
                                    quadrature,
                                    mapped_points);

  EXPECT_EQ(mapped_points.size(), 1);
  return mapped_points.at(0);
}

void
expectPointNear(const Point & actual, const Point & expected, const Real tolerance = 1e-10)
{
  for (const auto component : make_range(unsigned(LIBMESH_DIM)))
    EXPECT_NEAR(actual(component), expected(component), tolerance);
}
}

TEST(MortarProjectionTest, DistortedQuadSelectsInDomainRootAtAllScales)
{
  const std::vector<Point> base_points = {Point(1.105656610717944, -2.374361871447584, 0),
                                          Point(1.421255625312365, 1.7157664687565681, 0),
                                          Point(-0.4663591280150303, 1.5483600798486492, 0),
                                          Point(0.6300030586204182, -1.295788347267608, 0)};
  const Point base_target(1.0865728771728467, -2.3242274706887325, 0);
  const Point expected(-0.9970323331851889, -0.9181493063418376);

  for (const Real scale : {1e-9, 1.0, 1e9})
  {
    const Point translation = scale * Point(2.3, -1.7, 0.4);
    std::vector<Point> parent_points;
    for (const auto & point : base_points)
      parent_points.push_back(translation + scale * point);

    expectPointNear(
        projectPoint(QUAD4, parent_points, translation + scale * base_target, Point(0, 0, 1)),
        expected,
        2e-9);
  }
}

TEST(MortarProjectionTest, DirectTriangleAndAffineQuadrilateral)
{
  const std::vector<Point> triangle = {Point(0, 0, 0), Point(2, 0, 0), Point(0.2, 1.3, 0)};
  const Point triangle_reference(0.23, 0.31);
  expectPointNear(
      projectPoint(TRI3, triangle, mapPoint(TRI3, triangle, triangle_reference), Point(0, 0, 1)),
      triangle_reference);

  const std::vector<Point> quadrilateral = {
      Point(-1, -0.8, 0), Point(1.4, -0.8, 0), Point(1.4, 1.1, 0), Point(-1, 1.1, 0)};
  const Point quadrilateral_reference(0.17, -0.29);
  expectPointNear(projectPoint(QUAD4,
                               quadrilateral,
                               mapPoint(QUAD4, quadrilateral, quadrilateral_reference),
                               Point(0, 0, 1),
                               0,
                               true),
                  quadrilateral_reference);
}

TEST(MortarProjectionTest, ClippingToleranceControlsBoundarySnapping)
{
  const std::vector<Point> high_aspect_quad = {
      Point(-1, -5e-7, 0), Point(1, -5e-7, 0), Point(1, 5e-7, 0), Point(-1, 5e-7, 0)};
  MortarSegmentHelper helper(
      high_aspect_quad, Point(), Point(0, 0, 1), MortarSegmentTriangulationMode::Centroid, false);
  const Real clipping_area_tolerance = helper.areaTolerance();
  EXPECT_NEAR(clipping_area_tolerance, 2e-14, 1e-26);

  const Point tolerance_sized_reference(0.2, 1 + 1.5e-8);
  expectPointNear(projectPoint(QUAD4,
                               high_aspect_quad,
                               mapPoint(QUAD4, high_aspect_quad, tolerance_sized_reference),
                               Point(0, 0, 1),
                               clipping_area_tolerance),
                  Point(0.2, 1),
                  1e-10);

  const Point excessive_reference(0.2, 1.1);
  EXPECT_THROW(projectPoint(QUAD4,
                            high_aspect_quad,
                            mapPoint(QUAD4, high_aspect_quad, excessive_reference),
                            Point(0, 0, 1),
                            clipping_area_tolerance),
               MooseException);
}

TEST(MortarProjectionTest, InvalidGeometryAndClippingData)
{
  const std::vector<Point> singular_quad = {
      Point(0, -1, 0), Point(0, -0.2, 0), Point(0, 1, 0), Point(0, 0.2, 0)};
  EXPECT_THROW(projectPoint(QUAD4, singular_quad, Point(), Point(0, 0, 1)), MooseException);

  const std::vector<Point> folded_quad = {
      Point(-1, -1, 0), Point(1, 1, 0), Point(1, -1, 0), Point(-1, 1, 0)};
  EXPECT_THROW(projectPoint(QUAD4, folded_quad, Point(), Point(0, 0, 1)), MooseException);

  const std::vector<Point> secondary_nodes = {
      Point(-1, -1, 0), Point(1, -1, 0), Point(1, 1, 0), Point(-1, 1, 0)};
  const Point supplied_normal(0, 0, 3);
  MortarSegmentHelper helper(
      secondary_nodes, Point(), supplied_normal, MortarSegmentTriangulationMode::Centroid, false);
  EXPECT_EQ(helper.normal(), supplied_normal);
  EXPECT_NEAR(helper.areaTolerance(), 4e-8, 1e-20);
}
