//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "MooseMesh.h"
#include "OrientedBoundingBox.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/enum_elem_type.h"

#include <cmath>
#include <cstdio>
#include <vector>

using namespace libMesh;

namespace
{
/// True if `target` matches some point in `pts` within `tol`.
bool
containsPoint(const std::vector<Point> & pts, const Point & target, const Real tol)
{
  for (const auto & p : pts)
    if ((p - target).norm() <= tol)
      return true;
  return false;
}

/// Collect all node locations of a mesh (order is not preserved by Exodus I/O).
std::vector<Point>
nodeLocations(const MeshBase & mesh)
{
  std::vector<Point> pts;
  for (const auto * node : mesh.node_ptr_range())
    pts.push_back(*node);
  return pts;
}
}

// Write an oriented 3D box, read it back, and verify all eight corners land where
// the origin + subset-of-(len_i * dir_i) mapping predicts.
TEST(OrientedBoundingBoxTest, WriteMeshBox3D)
{
  libMesh::Parallel::Communicator comm(MPI_COMM_SELF);
  const Real tol = 1e-8;

  // Orthonormal basis rotated 45 deg in the xy-plane, z unchanged.
  const Real s = 1.0 / std::sqrt(2.0);
  const Point origin(10.0, 20.0, 30.0);
  const Point d0(s, s, 0.0), d1(-s, s, 0.0), d2(0.0, 0.0, 1.0);
  const Real l0 = 2.0, l1 = 3.0, l2 = 1.0;

  const std::vector<std::pair<Point, Point>> axis_pairs{
      {origin, origin + l0 * d0}, {origin, origin + l1 * d1}, {origin, origin + l2 * d2}};
  OrientedBoundingBox obb(axis_pairs);

  const std::string file = "oriented_bounding_box_test_3d.e";
  obb.writeMesh(file, comm);

  ReplicatedMesh mesh(comm);
  mesh.read(file);

  EXPECT_EQ(mesh.n_nodes(), 8u);
  EXPECT_EQ(mesh.n_elem(), 1u);
  for (const auto * elem : mesh.active_element_ptr_range())
    EXPECT_EQ(elem->type(), HEX8);

  const std::vector<Point> nodes = nodeLocations(mesh);
  for (unsigned int mask = 0; mask < 8u; ++mask)
  {
    Point corner = origin;
    if (mask & 1u)
      corner += l0 * d0;
    if (mask & 2u)
      corner += l1 * d1;
    if (mask & 4u)
      corner += l2 * d2;
    EXPECT_TRUE(containsPoint(nodes, corner, tol)) << "missing corner, mask=" << mask;
  }

  std::remove(file.c_str());
}

// Write an oriented 2D box, read it back, and verify all four corners.
TEST(OrientedBoundingBoxTest, WriteMeshBox2D)
{
  libMesh::Parallel::Communicator comm(MPI_COMM_SELF);
  const Real tol = 1e-8;

  const Real s = 1.0 / std::sqrt(2.0);
  const Point origin(5.0, 6.0, 0.0);
  const Point d0(s, s, 0.0), d1(-s, s, 0.0);
  const Real l0 = 2.0, l1 = 3.0;

  const std::vector<std::pair<Point, Point>> axis_pairs{{origin, origin + l0 * d0},
                                                        {origin, origin + l1 * d1}};
  OrientedBoundingBox obb(axis_pairs);

  const std::string file = "oriented_bounding_box_test_2d.e";
  obb.writeMesh(file, comm);

  ReplicatedMesh mesh(comm);
  mesh.read(file);

  EXPECT_EQ(mesh.n_nodes(), 4u);
  EXPECT_EQ(mesh.n_elem(), 1u);
  for (const auto * elem : mesh.active_element_ptr_range())
    EXPECT_EQ(elem->type(), QUAD4);

  const std::vector<Point> nodes = nodeLocations(mesh);
  for (unsigned int mask = 0; mask < 4u; ++mask)
  {
    Point corner = origin;
    if (mask & 1u)
      corner += l0 * d0;
    if (mask & 2u)
      corner += l1 * d1;
    EXPECT_TRUE(containsPoint(nodes, corner, tol)) << "missing corner, mask=" << mask;
  }

  std::remove(file.c_str());
}

// Write the ray along the shortest axis, read it back, and verify its endpoints.
TEST(OrientedBoundingBoxTest, WriteRay)
{
  libMesh::Parallel::Communicator comm(MPI_COMM_SELF);
  const Real tol = 1e-8;

  const Real s = 1.0 / std::sqrt(2.0);
  const Point origin(0.0, 0.0, 0.0);
  const Point d0(s, s, 0.0), d1(-s, s, 0.0), d2(0.0, 0.0, 1.0);
  const Real l0 = 2.0, l1 = 3.0, l2 = 1.0; // shortest axis is axis 2

  const std::vector<std::pair<Point, Point>> axis_pairs{
      {origin, origin + l0 * d0}, {origin, origin + l1 * d1}, {origin, origin + l2 * d2}};
  OrientedBoundingBox obb(axis_pairs);

  const std::string file = "oriented_bounding_box_test_ray.e";
  obb.writeRayAlongShortestAxis(file, comm);

  ReplicatedMesh mesh(comm);
  mesh.read(file);

  EXPECT_EQ(mesh.n_nodes(), 2u);
  EXPECT_EQ(mesh.n_elem(), 1u);
  for (const auto * elem : mesh.active_element_ptr_range())
    EXPECT_EQ(elem->type(), EDGE2);

  // Ray starts at the face centre orthogonal to the shortest axis and spans it.
  const Point start = origin + 0.5 * l0 * d0 + 0.5 * l1 * d1;
  const Point end = start + l2 * d2;

  const std::vector<Point> nodes = nodeLocations(mesh);
  EXPECT_TRUE(containsPoint(nodes, start, tol));
  EXPECT_TRUE(containsPoint(nodes, end, tol));

  std::remove(file.c_str());
}
