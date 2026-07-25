//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurfaceElementSetTest.h"
#include "MooseUnitUtils.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_quad4.h"
#include "libmesh/edge_edge2.h"

#include "gtest/gtest.h"

using namespace libMesh;

namespace
{

void
addTri(MeshBase & mesh, Node * n0, Node * n1, Node * n2)
{
  Elem * e = mesh.add_elem(new Tri3);
  e->set_node(0, n0);
  e->set_node(1, n1);
  e->set_node(2, n2);
}

// A unit square in the z=0 plane split into two Tri3 elements. Node bounding box
// is [0,1] x [0,1] x {0}; per-element centroids are the triangle vertex averages.
std::unique_ptr<ReplicatedMesh>
makeTwoTriangleMesh(const Parallel::Communicator & comm)
{
  auto mesh = std::make_unique<ReplicatedMesh>(comm);
  mesh->set_mesh_dimension(2);
  mesh->set_spatial_dimension(3);

  Node * n0 = mesh->add_point(Point(0, 0, 0));
  Node * n1 = mesh->add_point(Point(1, 0, 0));
  Node * n2 = mesh->add_point(Point(1, 1, 0));
  Node * n3 = mesh->add_point(Point(0, 1, 0));

  addTri(*mesh, n0, n1, n3); // centroid (1/3, 1/3, 0)
  addTri(*mesh, n1, n2, n3); // centroid (2/3, 2/3, 0)

  mesh->prepare_for_use();
  return mesh;
}

} // namespace

TEST_F(SurfaceElementSetTest, fromMesh)
{
  auto mesh = makeTwoTriangleMesh(_app->comm());
  const auto set = SurfaceElementSet::fromMesh(*mesh);

  ASSERT_EQ(set.size(), std::size_t{2});
  ASSERT_EQ(set.elements().size(), std::size_t{2});
  ASSERT_EQ(set.centroids().size(), std::size_t{2});

  // Centroids align index-for-index with elements (element iteration order).
  for (const auto i : index_range(set.elements()))
    EXPECT_TRUE(set.centroids()[i].absolute_fuzzy_equals(set.elements()[i]->elem().vertex_average(),
                                                         1e-12));

  // AABB is the union of the element node bounding boxes.
  const auto & bb = set.boundingBox();
  EXPECT_NEAR(bb.min()(0), 0.0, 1e-8);
  EXPECT_NEAR(bb.min()(1), 0.0, 1e-8);
  EXPECT_NEAR(bb.min()(2), 0.0, 1e-8);
  EXPECT_NEAR(bb.max()(0), 1.0, 1e-8);
  EXPECT_NEAR(bb.max()(1), 1.0, 1e-8);
  EXPECT_NEAR(bb.max()(2), 0.0, 1e-8);
}

TEST_F(SurfaceElementSetTest, fromElements)
{
  auto mesh = makeTwoTriangleMesh(_app->comm());

  // Wrap only the first active element as a subset.
  std::vector<const Elem *> elems;
  elems.push_back(*mesh->active_elements_begin());

  const auto set = SurfaceElementSet::fromElements(elems);

  ASSERT_EQ(set.size(), std::size_t{1});
  EXPECT_TRUE(set.centroids()[0].absolute_fuzzy_equals(elems[0]->vertex_average(), 1e-12));
  EXPECT_TRUE(set.elements()[0]->elem().type() == TRI3);
}

TEST_F(SurfaceElementSetTest, rejectsUnsupportedElementType)
{
  // A Quad4 face is not a supported surface element (only EDGE2 and TRI3 are).
  std::unique_ptr<Node> n0(new Node(Point(0, 0, 0), 0));
  std::unique_ptr<Node> n1(new Node(Point(1, 0, 0), 1));
  std::unique_ptr<Node> n2(new Node(Point(1, 1, 0), 2));
  std::unique_ptr<Node> n3(new Node(Point(0, 1, 0), 3));
  std::unique_ptr<Quad4> quad(new Quad4());
  quad->set_node(0, n0.get());
  quad->set_node(1, n1.get());
  quad->set_node(2, n2.get());
  quad->set_node(3, n3.get());

  std::vector<const Elem *> elems{quad.get()};
  EXPECT_MOOSEERROR_MSG_CONTAINS(SurfaceElementSet::fromElements(elems),
                                 "unsupported element type");
}

TEST_F(SurfaceElementSetTest, rejectsMixedElementTypes)
{
  // A group must be homogeneous: an EDGE2 followed by a TRI3 must be rejected.
  std::unique_ptr<Node> e0(new Node(Point(0, 0, 0), 0));
  std::unique_ptr<Node> e1(new Node(Point(1, 0, 0), 1));
  std::unique_ptr<Edge2> edge(new Edge2());
  edge->set_node(0, e0.get());
  edge->set_node(1, e1.get());

  std::unique_ptr<Node> t0(new Node(Point(0, 0, 0), 0));
  std::unique_ptr<Node> t1(new Node(Point(1, 0, 0), 1));
  std::unique_ptr<Node> t2(new Node(Point(0, 1, 0), 2));
  std::unique_ptr<Tri3> tri(new Tri3());
  tri->set_node(0, t0.get());
  tri->set_node(1, t1.get());
  tri->set_node(2, t2.get());

  std::vector<const Elem *> elems{edge.get(), tri.get()};
  EXPECT_MOOSEERROR_MSG_CONTAINS(SurfaceElementSet::fromElements(elems), "mixed element types");
}
