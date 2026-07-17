//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MortarSegmentHelper.h"
#include "MortarUtils.h"

#include "libmesh/elem.h"

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <vector>

using namespace libMesh;

class MortarSegmentHelperTest : public ::testing::Test
{
protected:
  static MortarSegmentHelper
  helper(const std::vector<Point> & nodes,
         const MortarSegmentTriangulationMode mode = MortarSegmentTriangulationMode::Centroid,
         const bool triangulate_triangles = false,
         const bool with_reference_points = false)
  {
    Point center;
    for (const auto & node : nodes)
      center += node;
    center /= nodes.size();

    if (with_reference_points)
      return MortarSegmentHelper(
          nodes, nodes, center, Point(0., 0., 1.), mode, triangulate_triangles);

    return MortarSegmentHelper(nodes, center, Point(0., 0., 1.), mode, triangulate_triangles);
  }

  static std::optional<Point> referencePoint(MortarSegmentHelper & helper,
                                             const Point & point,
                                             const std::vector<Point> & polygon,
                                             const std::vector<Point> & parent_reference_points,
                                             std::string * const failure_reason = nullptr)
  {
    return helper.referencePoint(point, polygon, parent_reference_points, failure_reason);
  }

  static Point bilinearPoint(const std::vector<Point> & polygon, const Real xi, const Real eta)
  {
    const std::array<Real, 4> shape = {{0.25 * (1. - xi) * (1. - eta),
                                        0.25 * (1. + xi) * (1. - eta),
                                        0.25 * (1. + xi) * (1. + eta),
                                        0.25 * (1. - xi) * (1. + eta)}};
    Point point;
    for (const auto i : index_range(shape))
      point += shape[i] * polygon[i];
    return point;
  }

  static void expectPointsNear(const std::vector<Point> & first,
                               const std::vector<Point> & second,
                               const Real tolerance = 1e-13)
  {
    ASSERT_EQ(first.size(), second.size());
    for (const auto i : index_range(first))
      EXPECT_LE((first[i] - second[i]).norm(), tolerance) << "point index " << i;
  }
};

TEST_F(MortarSegmentHelperTest, triangleReferenceRecoverySnapsOnlyToleranceViolations)
{
  const std::vector<Point> triangle = {Point(0., 0.), Point(1., 0.), Point(0., 1.)};
  auto segment_helper = helper(triangle);

  std::string reason;
  const auto snapped =
      referencePoint(segment_helper, Point(-0.5e-8, 0.25), triangle, triangle, &reason);
  ASSERT_TRUE(snapped) << reason;
  EXPECT_DOUBLE_EQ((*snapped)(0), 0.);
  EXPECT_NEAR((*snapped)(1), 0.25, 1e-8);

  reason.clear();
  const auto outside =
      referencePoint(segment_helper, Point(-2.e-8, 0.25), triangle, triangle, &reason);
  EXPECT_FALSE(outside);
  EXPECT_NE(reason.find("exceed"), std::string::npos) << reason;

  const std::vector<Point> ill_conditioned = {Point(0., 0.), Point(1., 0.), Point(0., 1.e-12)};
  reason.clear();
  const auto ill_conditioned_result =
      referencePoint(segment_helper, Point(0.25, 0.25e-12), ill_conditioned, triangle, &reason);
  EXPECT_FALSE(ill_conditioned_result);
  EXPECT_NE(reason.find("ill-conditioned"), std::string::npos) << reason;

  const std::vector<Point> coarse_triangle = {Point(0., 0.), Point(10., 0.), Point(0., 10.)};
  auto coarse_helper = helper(coarse_triangle);
  reason.clear();
  const auto clipping_sized_violation =
      referencePoint(coarse_helper, Point(-2.5e-7, 0.25), triangle, triangle, &reason);
  ASSERT_TRUE(clipping_sized_violation) << reason;
  EXPECT_DOUBLE_EQ((*clipping_sized_violation)(0), 0.);
}

TEST_F(MortarSegmentHelperTest, quadrilateralReferenceRecovery)
{
  const std::vector<Point> master = {
      Point(-1., -1.), Point(1., -1.), Point(1., 1.), Point(-1., 1.)};
  const std::vector<Point> distorted = {
      Point(0., 0.), Point(1., 0.), Point(1. + 1.e-4, 1.e-5), Point(0., 1.)};
  auto segment_helper = helper(master);

  constexpr Real xi = 0.8;
  constexpr Real eta = -0.9;
  std::string reason;
  const auto recovered =
      referencePoint(segment_helper, bilinearPoint(distorted, xi, eta), distorted, master, &reason);
  ASSERT_TRUE(recovered) << reason;
  EXPECT_NEAR((*recovered)(0), xi, 1e-8);
  EXPECT_NEAR((*recovered)(1), eta, 1e-8);

  const std::vector<Point> reversed = {Point(0., 0.), Point(0., 1.), Point(1., 1.), Point(1., 0.)};
  reason.clear();
  const auto reversed_recovered =
      referencePoint(segment_helper, bilinearPoint(reversed, xi, eta), reversed, master, &reason);
  ASSERT_TRUE(reversed_recovered) << reason;
  EXPECT_NEAR((*reversed_recovered)(0), xi, 1e-8);
  EXPECT_NEAR((*reversed_recovered)(1), eta, 1e-8);

  const std::vector<Point> coarse = {
      Point(0., 0.), Point(10., 0.), Point(10., 10.), Point(0., 10.)};
  const std::vector<Point> unit_square = {
      Point(0., 0.), Point(1., 0.), Point(1., 1.), Point(0., 1.)};
  auto coarse_helper = helper(coarse);
  reason.clear();
  const auto clipping_sized_violation =
      referencePoint(coarse_helper, Point(-5.e-7, 0.25), unit_square, master, &reason);
  ASSERT_TRUE(clipping_sized_violation) << reason;
  EXPECT_DOUBLE_EQ((*clipping_sized_violation)(0), -1.);
}

TEST_F(MortarSegmentHelperTest, quadrilateralReferenceRecoveryRejectsInvalidMaps)
{
  const std::vector<Point> master = {
      Point(-1., -1.), Point(1., -1.), Point(1., 1.), Point(-1., 1.)};
  auto segment_helper = helper(master);
  const std::array<std::vector<Point>, 3> invalid_polygons = {
      {{Point(0., 0.), Point(1., 0.), Point(2., 0.), Point(3., 0.)},
       {Point(0., 0.), Point(1., 0.), Point(0.25, 0.25), Point(0., 1.)},
       {Point(0., 0.), Point(1., 1.), Point(0., 1.), Point(1., 0.)}}};

  for (const auto & polygon : invalid_polygons)
  {
    std::string reason;
    EXPECT_FALSE(referencePoint(segment_helper, Point(0.25, 0.25), polygon, master, &reason));
    EXPECT_FALSE(reason.empty());
  }
}

TEST_F(MortarSegmentHelperTest, mappingDataDoesNotChangeSegmentGeometry)
{
  const std::vector<Point> secondary = {
      Point(-1., -1.), Point(1., -1.), Point(1., 1.), Point(-1., 1.)};
  const std::vector<Point> primary = {
      Point(-0.8, -0.9), Point(1.2, -0.7), Point(1.1, 0.8), Point(-0.7, 1.2)};
  const std::vector<MortarSegmentTriangulationMode> modes = {
      MortarSegmentTriangulationMode::Vertex,
      MortarSegmentTriangulationMode::Centroid,
      MortarSegmentTriangulationMode::EarClipping,
#if defined(LIBMESH_HAVE_TRIANGLE) || defined(LIBMESH_HAVE_POLY2TRI)
      MortarSegmentTriangulationMode::Delaunay,
#endif
  };

  for (const auto mode : modes)
  {
    auto normal_helper = helper(secondary, mode);
    auto reference_helper = helper(secondary, mode, false, true);
    std::vector<Point> normal_nodes;
    std::vector<Point> reference_nodes;
    std::vector<std::vector<unsigned int>> normal_elements;
    std::vector<std::vector<unsigned int>> reference_elements;
    std::vector<std::array<Point, 3>> secondary_reference_points;
    std::vector<std::array<Point, 3>> primary_reference_points;

    normal_helper.getMortarSegments(primary, normal_nodes, normal_elements);
    reference_helper.getMortarSegments(primary,
                                       primary,
                                       reference_nodes,
                                       reference_elements,
                                       secondary_reference_points,
                                       primary_reference_points);

    expectPointsNear(normal_nodes, reference_nodes);
    EXPECT_EQ(normal_elements, reference_elements);
    EXPECT_EQ(reference_elements.size(), secondary_reference_points.size());
    EXPECT_EQ(reference_elements.size(), primary_reference_points.size());
  }
}

TEST_F(MortarSegmentHelperTest, referenceInterpolationMapsParallelSurfaceQuadraturePoints)
{
  const std::vector<Point> master = {
      Point(-1., -1.), Point(1., -1.), Point(1., 1.), Point(-1., 1.)};
  const std::vector<Point> secondary = {
      Point(-1., -1., 0.), Point(1., -1., 0.), Point(1., 1., 0.), Point(-1., 1., 0.)};

  std::vector<Point> primary;
  primary.reserve(master.size());
  for (const auto & point : master)
    primary.emplace_back(
        0.2 + 0.8 * point(0) - 0.3 * point(1), -0.1 + 0.2 * point(0) + 0.7 * point(1), 0.25);

  // Supply the primary nodes clockwise to exercise the orientation correction and its matching
  // reordering of parent reference points.
  std::vector<Point> reversed_primary = primary;
  std::vector<Point> reversed_primary_reference_points = master;
  std::reverse(reversed_primary.begin(), reversed_primary.end());
  std::reverse(reversed_primary_reference_points.begin(), reversed_primary_reference_points.end());

  const std::vector<MortarSegmentTriangulationMode> modes = {
      MortarSegmentTriangulationMode::Vertex,
      MortarSegmentTriangulationMode::Centroid,
      MortarSegmentTriangulationMode::EarClipping,
#if defined(LIBMESH_HAVE_TRIANGLE) || defined(LIBMESH_HAVE_POLY2TRI)
      MortarSegmentTriangulationMode::Delaunay,
#endif
  };

  for (const auto mode : modes)
  {
    MortarSegmentHelper reference_helper(
        secondary, master, Point(), Point(0., 0., 1.), mode, false);
    std::vector<Point> mortar_nodes;
    std::vector<std::vector<unsigned int>> mortar_elements;
    std::vector<std::array<Point, 3>> secondary_reference_points;
    std::vector<std::array<Point, 3>> primary_reference_points;
    reference_helper.getMortarSegments(reversed_primary,
                                       reversed_primary_reference_points,
                                       mortar_nodes,
                                       mortar_elements,
                                       secondary_reference_points,
                                       primary_reference_points);

    ASSERT_FALSE(mortar_elements.empty());
    ASSERT_EQ(mortar_elements.size(), secondary_reference_points.size());
    ASSERT_EQ(mortar_elements.size(), primary_reference_points.size());
    for (const auto elem_index : index_range(mortar_elements))
    {
      Point mortar_qp;
      Point secondary_reference_qp;
      Point primary_reference_qp;
      for (const auto local_node : make_range(3))
      {
        mortar_qp += mortar_nodes[mortar_elements[elem_index][local_node]] / 3.;
        secondary_reference_qp += secondary_reference_points[elem_index][local_node] / 3.;
        primary_reference_qp += primary_reference_points[elem_index][local_node] / 3.;
      }

      const Point secondary_qp =
          bilinearPoint(secondary, secondary_reference_qp(0), secondary_reference_qp(1));
      const Point primary_qp =
          bilinearPoint(primary, primary_reference_qp(0), primary_reference_qp(1));

      EXPECT_NEAR(secondary_qp(0), mortar_qp(0), 1e-12);
      EXPECT_NEAR(secondary_qp(1), mortar_qp(1), 1e-12);
      EXPECT_NEAR(secondary_qp(2), mortar_qp(2), 1e-12);
      EXPECT_NEAR(primary_qp(0), mortar_qp(0), 1e-12);
      EXPECT_NEAR(primary_qp(1), mortar_qp(1), 1e-12);
      EXPECT_NEAR(primary_qp(2) - mortar_qp(2), 0.25, 1e-12);
    }
  }
}

TEST_F(MortarSegmentHelperTest, centroidAndTriangleRetessellationCarryReferencePoints)
{
  const std::vector<Point> triangle = {Point(0., 0.), Point(1., 0.), Point(0., 1.)};

  for (const auto mode : {MortarSegmentTriangulationMode::Vertex,
                          MortarSegmentTriangulationMode::Centroid,
                          MortarSegmentTriangulationMode::EarClipping,
                          MortarSegmentTriangulationMode::Delaunay})
  {
    auto normal_helper = helper(triangle, mode, true);
    auto reference_helper = helper(triangle, mode, true, true);
    std::vector<Point> normal_nodes;
    std::vector<Point> reference_nodes;
    std::vector<std::vector<unsigned int>> normal_elements;
    std::vector<std::vector<unsigned int>> reference_elements;
    std::vector<std::array<Point, 3>> secondary_reference_points;
    std::vector<std::array<Point, 3>> primary_reference_points;

    normal_helper.getMortarSegments(triangle, normal_nodes, normal_elements);
    reference_helper.getMortarSegments(triangle,
                                       triangle,
                                       reference_nodes,
                                       reference_elements,
                                       secondary_reference_points,
                                       primary_reference_points);

    expectPointsNear(normal_nodes, reference_nodes);
    EXPECT_EQ(normal_elements, reference_elements);
    EXPECT_EQ(normal_nodes.size(), 4);
    EXPECT_EQ(normal_elements.size(), 3);
    EXPECT_EQ(reference_elements.size(), secondary_reference_points.size());
    EXPECT_EQ(reference_elements.size(), primary_reference_points.size());
  }

  auto filtered_helper = helper(triangle, MortarSegmentTriangulationMode::Centroid, true, true);
  std::vector<Point> filtered_nodes;
  std::vector<std::vector<unsigned int>> filtered_elements;
  std::vector<std::array<Point, 3>> filtered_secondary_reference_points;
  std::vector<std::array<Point, 3>> filtered_primary_reference_points;
  filtered_helper.getMortarSegments(triangle,
                                    triangle,
                                    filtered_nodes,
                                    filtered_elements,
                                    filtered_secondary_reference_points,
                                    filtered_primary_reference_points,
                                    1.);
  EXPECT_TRUE(filtered_nodes.empty());
  EXPECT_TRUE(filtered_elements.empty());
  EXPECT_TRUE(filtered_secondary_reference_points.empty());
  EXPECT_TRUE(filtered_primary_reference_points.empty());
}

TEST_F(MortarSegmentHelperTest, supportedParentFaceTopologies)
{
  struct ExpectedTopology
  {
    ElemType parent_type;
    std::vector<std::vector<unsigned int>> node_indices;
  };

  const std::vector<ExpectedTopology> expected = {
      {TRI3, {{0, 1, 2}}},
      {TRI6, {{0, 3, 5}, {3, 4, 5}, {3, 1, 4}, {5, 4, 2}}},
      {TRI7, {{0, 3, 5}, {3, 4, 5}, {3, 1, 4}, {5, 4, 2}}},
      {QUAD4, {{0, 1, 2, 3}}},
      {QUAD8, {{0, 4, 7}, {4, 1, 5}, {5, 2, 6}, {7, 6, 3}, {4, 5, 6, 7}}},
      {QUAD9, {{0, 4, 8, 7}, {4, 1, 5, 8}, {8, 5, 2, 6}, {7, 8, 6, 3}}}};

  for (const auto & expected_parent : expected)
  {
    const auto parent = Elem::build(expected_parent.parent_type);
    ASSERT_EQ(parent->n_sub_elem(), expected_parent.node_indices.size());
    for (const auto sub_element : make_range(parent->n_sub_elem()))
    {
      const auto node_indices = Moose::Mortar::getMortarSubElementNodeIndices(*parent, sub_element);
      EXPECT_EQ(node_indices, expected_parent.node_indices[sub_element]);
      for (const auto node : node_indices)
      {
        EXPECT_LT(node, parent->n_nodes());
        EXPECT_TRUE(parent->on_reference_element(parent->master_point(node), TOLERANCE));
      }
    }
  }
}
