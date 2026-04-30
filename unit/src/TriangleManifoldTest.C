//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriangleManifoldTest.h"
#include "MooseUnitUtils.h"

#include "libmesh/face_quad4.h"
#include "libmesh/face_tri3.h"
#include "libmesh/replicated_mesh.h"

#include "gtest/gtest.h"

namespace
{

void
addTri(libMesh::MeshBase & mesh, libMesh::Node * n0, libMesh::Node * n1, libMesh::Node * n2)
{
  libMesh::Elem * e = mesh.add_elem(new libMesh::Tri3);
  e->set_node(0, n0);
  e->set_node(1, n1);
  e->set_node(2, n2);
}

// Watertight unit cube [0,1]^3 as 12 Tri3 elements with outward normals.
// Nodes: n[0]=(0,0,0) n[1]=(1,0,0) n[2]=(1,1,0) n[3]=(0,1,0)
//        n[4]=(0,0,1) n[5]=(1,0,1) n[6]=(1,1,1) n[7]=(0,1,1)
template <typename MeshClass = libMesh::ReplicatedMesh>
std::unique_ptr<MeshClass>
makeUnitCubeMesh(const Parallel::Communicator & comm)
{
  auto mesh = std::make_unique<MeshClass>(comm);
  mesh->set_mesh_dimension(2);
  mesh->set_spatial_dimension(3);

  libMesh::Node * n[8];
  n[0] = mesh->add_point(Point(0, 0, 0));
  n[1] = mesh->add_point(Point(1, 0, 0));
  n[2] = mesh->add_point(Point(1, 1, 0));
  n[3] = mesh->add_point(Point(0, 1, 0));
  n[4] = mesh->add_point(Point(0, 0, 1));
  n[5] = mesh->add_point(Point(1, 0, 1));
  n[6] = mesh->add_point(Point(1, 1, 1));
  n[7] = mesh->add_point(Point(0, 1, 1));

  addTri(*mesh, n[0], n[2], n[1]); // bottom -z
  addTri(*mesh, n[0], n[3], n[2]);
  addTri(*mesh, n[4], n[5], n[6]); // top +z
  addTri(*mesh, n[4], n[6], n[7]);
  addTri(*mesh, n[0], n[1], n[5]); // front -y
  addTri(*mesh, n[0], n[5], n[4]);
  addTri(*mesh, n[2], n[3], n[7]); // back +y
  addTri(*mesh, n[2], n[7], n[6]);
  addTri(*mesh, n[0], n[4], n[7]); // left -x
  addTri(*mesh, n[0], n[7], n[3]);
  addTri(*mesh, n[1], n[2], n[6]); // right +x
  addTri(*mesh, n[1], n[6], n[5]);

  mesh->prepare_for_use();
  return mesh;
}

template std::unique_ptr<libMesh::ReplicatedMesh>
makeUnitCubeMesh<libMesh::ReplicatedMesh>(const Parallel::Communicator &);
#ifndef NDEBUG
template std::unique_ptr<libMesh::DistributedMesh>
makeUnitCubeMesh<libMesh::DistributedMesh>(const Parallel::Communicator &);
#endif

// Same cube with the +z face removed (open surface: 10 triangles, boundary edges exposed).
std::unique_ptr<libMesh::ReplicatedMesh>
makeOpenCubeMesh(const Parallel::Communicator & comm)
{
  auto mesh = std::make_unique<libMesh::ReplicatedMesh>(comm);
  mesh->set_mesh_dimension(2);
  mesh->set_spatial_dimension(3);

  libMesh::Node * n[8];
  n[0] = mesh->add_point(Point(0, 0, 0));
  n[1] = mesh->add_point(Point(1, 0, 0));
  n[2] = mesh->add_point(Point(1, 1, 0));
  n[3] = mesh->add_point(Point(0, 1, 0));
  n[4] = mesh->add_point(Point(0, 0, 1));
  n[5] = mesh->add_point(Point(1, 0, 1));
  n[6] = mesh->add_point(Point(1, 1, 1));
  n[7] = mesh->add_point(Point(0, 1, 1));

  addTri(*mesh, n[0], n[2], n[1]); // bottom -z
  addTri(*mesh, n[0], n[3], n[2]);
  // top face omitted
  addTri(*mesh, n[0], n[1], n[5]); // front -y
  addTri(*mesh, n[0], n[5], n[4]);
  addTri(*mesh, n[2], n[3], n[7]); // back +y
  addTri(*mesh, n[2], n[7], n[6]);
  addTri(*mesh, n[0], n[4], n[7]); // left -x
  addTri(*mesh, n[0], n[7], n[3]);
  addTri(*mesh, n[1], n[2], n[6]); // right +x
  addTri(*mesh, n[1], n[6], n[5]);

  mesh->prepare_for_use();
  return mesh;
}

// 2D mesh with a single Quad4 element (disallowed by SurfaceChecker).
std::unique_ptr<libMesh::ReplicatedMesh>
makeNonTri3Mesh(const Parallel::Communicator & comm)
{
  auto mesh = std::make_unique<libMesh::ReplicatedMesh>(comm);
  mesh->set_mesh_dimension(2);
  mesh->set_spatial_dimension(3);

  libMesh::Node * n0 = mesh->add_point(Point(0, 0, 0));
  libMesh::Node * n1 = mesh->add_point(Point(1, 0, 0));
  libMesh::Node * n2 = mesh->add_point(Point(1, 1, 0));
  libMesh::Node * n3 = mesh->add_point(Point(0, 1, 0));

  libMesh::Elem * e = mesh->add_elem(new libMesh::Quad4);
  e->set_node(0, n0);
  e->set_node(1, n1);
  e->set_node(2, n2);
  e->set_node(3, n3);

  mesh->prepare_for_use();
  return mesh;
}

// Empty 2D surface mesh (no elements).
std::unique_ptr<libMesh::ReplicatedMesh>
makeEmptyMesh(const Parallel::Communicator & comm)
{
  auto mesh = std::make_unique<libMesh::ReplicatedMesh>(comm);
  mesh->set_mesh_dimension(2);
  mesh->set_spatial_dimension(3);
  mesh->prepare_for_use();
  return mesh;
}

// Single Tri3 with collinear nodes: zero area, no neighbors, degenerate bounding box.
std::unique_ptr<libMesh::ReplicatedMesh>
makeDegenerateMesh(const Parallel::Communicator & comm)
{
  auto mesh = std::make_unique<libMesh::ReplicatedMesh>(comm);
  mesh->set_mesh_dimension(2);
  mesh->set_spatial_dimension(3);

  libMesh::Node * n0 = mesh->add_point(Point(0, 0, 0));
  libMesh::Node * n1 = mesh->add_point(Point(1, 0, 0));
  libMesh::Node * n2 = mesh->add_point(Point(2, 0, 0));

  libMesh::Elem * e = mesh->add_elem(new libMesh::Tri3);
  e->set_node(0, n0);
  e->set_node(1, n1);
  e->set_node(2, n2);

  mesh->prepare_for_use();
  return mesh;
}

} // namespace

TEST_F(TriangleManifoldTest, containmentBasic)
{
  auto mesh = makeUnitCubeMesh(_app->comm());
  TriangleManifold manifold(*mesh, 1e-10);

  EXPECT_EQ(manifold.numTriangles(), std::size_t{12});
  EXPECT_TRUE(manifold.contains(Point(0.5, 0.5, 0.5)));   // center
  EXPECT_FALSE(manifold.contains(Point(1.5, 0.5, 0.5)));  // outside +x
  EXPECT_FALSE(manifold.contains(Point(-0.5, 0.5, 0.5))); // outside -x
}

TEST_F(TriangleManifoldTest, surfacePoint)
{
  auto mesh = makeUnitCubeMesh(_app->comm());
  TriangleManifold manifold(*mesh, 1e-6);

  // A point exactly at a mesh vertex is detected as on-surface and counted as inside.
  EXPECT_TRUE(manifold.contains(Point(0.0, 0.0, 0.0)));
  // A point on the interior of a face is also on-surface.
  EXPECT_TRUE(manifold.contains(Point(0.5, 0.5, 0.0)));
}

TEST_F(TriangleManifoldTest, boundingBox)
{
  auto mesh = makeUnitCubeMesh(_app->comm());
  TriangleManifold manifold(*mesh, 1e-10);

  EXPECT_NEAR(manifold.boundingBox().min()(0), 0.0, 1e-12);
  EXPECT_NEAR(manifold.boundingBox().min()(1), 0.0, 1e-12);
  EXPECT_NEAR(manifold.boundingBox().min()(2), 0.0, 1e-12);
  EXPECT_NEAR(manifold.boundingBox().max()(0), 1.0, 1e-12);
  EXPECT_NEAR(manifold.boundingBox().max()(1), 1.0, 1e-12);
  EXPECT_NEAR(manifold.boundingBox().max()(2), 1.0, 1e-12);
}

TEST_F(TriangleManifoldTest, emptyMesh)
{
  auto mesh = makeEmptyMesh(_app->comm());
#ifdef NDEBUG
  EXPECT_MOOSEERROR_MSG_CONTAINS(TriangleManifold(*mesh, 1e-10), "surface mesh was empty");
#else
  EXPECT_THROW_MSG_CONTAINS(
      TriangleManifold(*mesh, 1e-10), std::runtime_error, "Manifold mesh must be a surface.");
#endif
}

TEST_F(TriangleManifoldTest, openSurface)
{
  auto mesh = makeOpenCubeMesh(_app->comm());
  EXPECT_MOOSEERROR_MSG_CONTAINS(TriangleManifold(*mesh, 1e-10),
                                 "triangle without three neighbors");
}

TEST_F(TriangleManifoldTest, nonTri3Element)
{
  auto mesh = makeNonTri3Mesh(_app->comm());
  EXPECT_MOOSEERROR_MSG_CONTAINS(TriangleManifold(*mesh, 1e-10), "non-Tri3 element");
}

TEST_F(TriangleManifoldTest, degenerateElement)
{
  auto mesh = makeDegenerateMesh(_app->comm());
  EXPECT_MOOSEERROR_MSG_CONTAINS(TriangleManifold(*mesh, 1e-10), "degenerate");
}

#ifndef NDEBUG
TEST_F(TriangleManifoldTest, negativeTolerance)
{
  auto mesh = makeUnitCubeMesh(_app->comm());
  EXPECT_THROW_MSG_CONTAINS(TriangleManifold(*mesh, -1e-10),
                            std::runtime_error,
                            "surface_tolerance must be strictly positive");
}

TEST_F(TriangleManifoldTest, distributedMesh)
{
  auto mesh = makeUnitCubeMesh<libMesh::DistributedMesh>(_app->comm());
  mesh->set_distributed();
  EXPECT_THROW_MSG_CONTAINS(TriangleManifold(*mesh, 1e-10),
                            std::runtime_error,
                            "Input manifold mesh must be serialized.");
}
#endif
