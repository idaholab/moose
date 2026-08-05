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
#include "libmesh/face_tri3.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/int_range.h"
#include "SBMUtils.h"
#include <libmesh/serial_mesh.h>

#include <array>
#include <utility>

using namespace libMesh;

namespace
{
// Add the points to the mesh (node id == index) and return the created nodes.
std::vector<Node *>
addNodes(MeshBase & mesh, const std::vector<Point> & points)
{
  std::vector<Node *> node_ptrs;
  for (const auto i : index_range(points))
    node_ptrs.push_back(mesh.add_point(points[i], i)); // Mesh owns the node
  return node_ptrs;
}

// Finalize the mesh (with neighbor info) and return SBMUtils' watertightness verdict.
bool
isWatertight(MeshBase & mesh)
{
  mesh.prepare_for_use(false);

  std::vector<const Elem *> raw_ptrs;
  for (const auto * el : mesh.active_local_element_ptr_range())
    raw_ptrs.push_back(el);
  return SBMUtils::checkWatertightnessFromRawElems(raw_ptrs);
}

// Build an EDGE2 loop from the given points and 2-node connectivity, then test watertightness.
bool
edgeLoopIsWatertight(const Parallel::Communicator & comm,
                     const std::vector<Point> & points,
                     const std::vector<std::pair<unsigned int, unsigned int>> & edges)
{
  SerialMesh mesh(comm);
  const auto nodes = addNodes(mesh, points);
  for (const auto & [id0, id1] : edges)
  {
    auto edge = new Edge2();
    edge->set_node(0, nodes[id0]);
    edge->set_node(1, nodes[id1]);
    mesh.add_elem(edge); // Mesh owns the element
  }
  return isWatertight(mesh);
}

// Build a TRI3 surface from the given points and 3-node connectivity, then test watertightness.
bool
triSurfaceIsWatertight(const Parallel::Communicator & comm,
                       const std::vector<Point> & points,
                       const std::vector<std::array<unsigned int, 3>> & faces)
{
  SerialMesh mesh(comm);
  const auto nodes = addNodes(mesh, points);
  for (const auto & f : faces)
  {
    auto tri = new Tri3();
    tri->set_node(0, nodes[f[0]]);
    tri->set_node(1, nodes[f[1]]);
    tri->set_node(2, nodes[f[2]]);
    mesh.add_elem(tri); // Mesh owns the element
  }
  return isWatertight(mesh);
}
}

// Unit square boundary: an open loop (missing the closing edge) is not watertight; the closed loop
// is.
TEST(WaterTightTest, TwoDGeoOpenAndClose)
{
  libMesh::Parallel::Communicator comm(MPI_COMM_SELF);
  const std::vector<Point> points = {
      Point(0.0, 0.0, 0.0), Point(1.0, 0.0, 0.0), Point(1.0, 1.0, 0.0), Point(0.0, 1.0, 0.0)};

  EXPECT_FALSE(edgeLoopIsWatertight(comm, points, {{0, 1}, {1, 2}, {2, 3}}));
  EXPECT_TRUE(edgeLoopIsWatertight(comm, points, {{0, 1}, {1, 2}, {2, 3}, {3, 0}}));
}

// Tetrahedron surface: three of the four triangular faces leave an opening (not watertight); all
// four close it.
TEST(WaterTightTest, ThreeDGeoOpenAndClose)
{
  libMesh::Parallel::Communicator comm(MPI_COMM_SELF);
  const std::vector<Point> points = {
      Point(0.0, 0.0, 0.0), Point(1.0, 0.0, 0.0), Point(1.0, 1.0, 0.0), Point(0.0, 1.0, 0.0)};

  EXPECT_FALSE(triSurfaceIsWatertight(comm, points, {{{0, 1, 2}}, {{0, 1, 3}}, {{1, 2, 3}}}));
  EXPECT_TRUE(
      triSurfaceIsWatertight(comm, points, {{{0, 1, 2}}, {{0, 1, 3}}, {{1, 2, 3}}, {{0, 2, 3}}}));
}
