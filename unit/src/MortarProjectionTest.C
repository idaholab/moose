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
#include "MooseException.h"
#include "MortarUtils.h"

#include "libmesh/elem.h"
#include "libmesh/replicated_mesh.h"

#include <array>
#include <vector>

using namespace libMesh;

namespace
{
Point
projectPoint(const ElemType parent_type,
             const std::vector<Point> & parent_points,
             const Point & physical_target,
             const unsigned int subpatch = 0,
             const Point & mortar_reference_point = Point(0.8, 0.1))
{
  Parallel::Communicator communicator;
  ReplicatedMesh mesh(communicator, 2);
  mesh.set_spatial_dimension(3);
  const auto subpatch_index = mesh.add_elem_integer("subpatch");

  auto parent = Elem::build(parent_type);
  for (const auto node : parent->node_index_range())
  {
    auto * const mesh_node = mesh.add_point(parent_points[node]);
    parent->set_node(node, mesh_node);
  }
  const Elem * const parent_ptr = mesh.add_elem(std::move(parent));

  // TRI3 weights (0.1, 0.8, 0.1) place this QP at the target; 1e-4 keeps it local.
  const Real segment_scale = (parent_points[1] - parent_points[0]).norm() * 1e-4;
  const Point first_offset(1, 0);
  const Point second_offset(-0.125, -0.125);
  // Solve for the third offset so alternate quadrature coordinates still map to the target.
  const Point third_offset =
      -((1 - mortar_reference_point(0) - mortar_reference_point(1)) * first_offset +
        mortar_reference_point(0) * second_offset) /
      mortar_reference_point(1);
  const std::array<Point, 3> segment_points = {physical_target + segment_scale * first_offset,
                                               physical_target + segment_scale * second_offset,
                                               physical_target + segment_scale * third_offset};
  auto segment = Elem::build(TRI3);
  for (const auto node : segment->node_index_range())
  {
    auto * const mesh_node = mesh.add_point(segment_points[node]);
    segment->set_node(node, mesh_node);
  }
  Elem * const segment_ptr = mesh.add_elem(std::move(segment));
  segment_ptr->set_extra_integer(subpatch_index, subpatch);

  ArbitraryQuadrature quadrature(2);
  quadrature.setPoints({mortar_reference_point});
  std::vector<Point> mapped_points;
  Moose::Mortar::projectQPoints3d(
      segment_ptr, parent_ptr, subpatch_index, quadrature, mapped_points);

  EXPECT_EQ(mapped_points.size(), 1);
  return mapped_points.at(0);
}

std::vector<Point>
masterPoints(const ElemType type)
{
  const auto elem = Elem::build(type);
  std::vector<Point> points;
  for (const auto node : elem->node_index_range())
    points.push_back(elem->master_point(node));
  return points;
}
}

TEST(MortarProjectionTest, DistortedQuadFallsBackToStrictInDomainRootAtAllScales)
{
  const std::vector<Point> base_points = {Point(-0.3532553670180527, -1.2387155771347915, 0),
                                          Point(0.3532553670180527, -0.7612844228652085, 0),
                                          Point(1.6467446329819473, 0.7612844228652085, 0),
                                          Point(-1.6467446329819473, 1.2387155771347915, 0)};
  const Point base_target(-0.28128828148350693, -1.162854243983928, 0);
  const Point expected(-0.7710050142105261, -0.982098182744595);

  for (const Real scale : {1e-9, 1.0, 1e9})
  {
    const Point translation = scale * Point(2.3, -1.7, 0.4);
    std::vector<Point> parent_points;
    for (const auto & point : base_points)
      parent_points.push_back(translation + scale * point);

    const Point actual = projectPoint(QUAD4, parent_points, translation + scale * base_target);
    // This covers scale/translation roundoff while tightly checking the recovered root.
    for (const auto component : make_range(unsigned(LIBMESH_DIM)))
      EXPECT_NEAR(actual(component), expected(component), 2e-9);
  }
}

TEST(MortarProjectionTest, ReverseEliminationRecoversInDomainRoot)
{
  const std::vector<Point> quadrilateral = {
      Point(-1, -0.5, 0), Point(1, -1.5, 0), Point(1, 1.5, 0), Point(-1, 0.5, 0)};
  const Point reference_point(0.2, 0.3);
  const Point physical_target(reference_point(0),
                              reference_point(1) * (1 + 0.5 * reference_point(0)));
  // The first eliminant also yields xi = -2, where reconstruction is singular. The chosen eta
  // makes the least-squares Newton update vanish there, forcing reverse elimination.
  const Point mortar_reference_point(-2, 2 * (-2 - reference_point(0)) / physical_target(1));
  const Point actual =
      projectPoint(QUAD4, quadrilateral, physical_target, 0, mortar_reference_point);

  // Match the production normalized inverse-residual tolerance.
  for (const auto component : make_range(unsigned(LIBMESH_DIM)))
    EXPECT_NEAR(actual(component), reference_point(component), 1e-10);
}

TEST(MortarProjectionTest, BoundaryOnlyQuadrilateralRootIsSnapped)
{
  const std::vector<Point> parent_points = {Point(-1, -1), Point(1, -1), Point(1, 1), Point(-1, 1)};
  // The analytical fallback admits this clipping-sized violation, then verifies the clamped root.
  const Point actual = projectPoint(QUAD4, parent_points, Point(1 + 5e-9, 0.25));

  EXPECT_DOUBLE_EQ(actual(0), 1);
  EXPECT_DOUBLE_EQ(actual(1), 0.25);

  EXPECT_THROW(projectPoint(QUAD4, parent_points, Point(1 + 2e-8, 0.25)), MooseException);
}

TEST(MortarProjectionTest, TriangleBoundaryViolationIsSnapped)
{
  const auto parent_points = masterPoints(TRI3);
  const Point actual = projectPoint(TRI3, parent_points, Point(0.6, 0.4 + 5e-9));

  EXPECT_DOUBLE_EQ(actual(0) + actual(1), 1);
  EXPECT_DOUBLE_EQ(actual(0), 0.6 / (1 + 5e-9));
  EXPECT_DOUBLE_EQ(actual(1), (0.4 + 5e-9) / (1 + 5e-9));
}

TEST(MortarProjectionTest, TriangleSubpatchRejectsPointOutsideLocalDomain)
{
  // These targets remain inside the complete parent face but lie outside subpatch zero.
  for (const auto parent_type : {TRI6, TRI7})
    EXPECT_THROW(projectPoint(parent_type, masterPoints(parent_type), Point(0.4, 0.4)),
                 MooseException);

  EXPECT_THROW(projectPoint(QUAD8, masterPoints(QUAD8), Point(-0.2, -0.2)), MooseException);
}
