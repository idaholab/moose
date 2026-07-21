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
#include "SBMUtils.h"
#include <libmesh/serial_mesh.h>

using namespace libMesh;

TEST(WaterTightTest, TwoDGeoOpenAndClose)
{
  libMesh::Parallel::Communicator comm(MPI_COMM_SELF);
  std::vector<Point> points = {
      Point(0.0, 0.0, 0.0), // p0
      Point(1.0, 0.0, 0.0), // p1
      Point(1.0, 1.0, 0.0), // p2
      Point(0.0, 1.0, 0.0)  // p3
  };

  // (a) Test open geometry (missing last edge)
  {
    auto mesh = std::make_unique<libMesh::SerialMesh>(comm);
    std::vector<Node *> node_ptrs;

    for (unsigned int i = 0; i < points.size(); ++i)
      node_ptrs.push_back(mesh->add_point(points[i], i)); // Mesh owns the node

    std::vector<std::pair<unsigned int, unsigned int>> open_edges = {{0, 1}, {1, 2}, {2, 3}};
    for (const auto & [id1, id2] : open_edges)
    {
      auto edge = new Edge2();
      edge->set_node(0) = node_ptrs[id1];
      edge->set_node(1) = node_ptrs[id2];
      mesh->add_elem(edge); // Mesh owns the element
    }

    mesh->prepare_for_use(false); // Neighbor info setup

    std::vector<const Elem *> raw_ptrs;
    for (const auto * el : mesh->active_local_element_ptr_range())
      raw_ptrs.push_back(el);

    EXPECT_FALSE(SBMUtils::checkWatertightnessFromRawElems(raw_ptrs));
  }

  // (b) Test closed geometry
  {
    auto mesh = std::make_unique<libMesh::SerialMesh>(comm);
    std::vector<Node *> node_ptrs;

    for (unsigned int i = 0; i < points.size(); ++i)
      node_ptrs.push_back(mesh->add_point(points[i], i)); // Mesh owns the node

    std::vector<std::pair<unsigned int, unsigned int>> closed_edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}};
    for (const auto & [id1, id2] : closed_edges)
    {
      auto edge = new Edge2();
      edge->set_node(0) = node_ptrs[id1];
      edge->set_node(1) = node_ptrs[id2];
      mesh->add_elem(edge); // Mesh owns the element
    }

    mesh->prepare_for_use(false); // Neighbor info setup

    std::vector<const Elem *> raw_ptrs;
    for (const auto * el : mesh->active_local_element_ptr_range())
      raw_ptrs.push_back(el);

    EXPECT_TRUE(SBMUtils::checkWatertightnessFromRawElems(raw_ptrs));
  }
}

TEST(WaterTightTest, ThreeDGeoOpenAndClose)
{
  libMesh::Parallel::Communicator comm(MPI_COMM_SELF);
  std::vector<Point> points = {
      Point(0.0, 0.0, 0.0), // p0
      Point(1.0, 0.0, 0.0), // p1
      Point(1.0, 1.0, 0.0), // p2
      Point(0.0, 1.0, 0.0)  // p3
  };

  // (a) Test open geometry (missing last edge)
  {
    auto mesh = std::make_unique<libMesh::SerialMesh>(comm);
    std::vector<Node *> node_ptrs;

    for (unsigned int i = 0; i < points.size(); ++i)
      node_ptrs.push_back(mesh->add_point(points[i], i)); // Mesh owns the node

    std::vector<std::tuple<unsigned int, unsigned int, unsigned int>> open_faces = {
        {0, 1, 2}, {0, 1, 3}, {1, 2, 3}};
    for (const auto & [id1, id2, id3] : open_faces)
    {
      auto tri = new Tri3();
      tri->set_node(0) = node_ptrs[id1];
      tri->set_node(1) = node_ptrs[id2];
      tri->set_node(2) = node_ptrs[id3];
      mesh->add_elem(tri); // Mesh owns the element
    }

    mesh->prepare_for_use(false); // Neighbor info setup

    std::vector<const Elem *> raw_ptrs;
    for (const auto * el : mesh->active_local_element_ptr_range())
      raw_ptrs.push_back(el);

    EXPECT_FALSE(SBMUtils::checkWatertightnessFromRawElems(raw_ptrs));
  }

  // (b) Test closed geometry
  {
    auto mesh = std::make_unique<libMesh::SerialMesh>(comm);
    std::vector<Node *> node_ptrs;

    for (unsigned int i = 0; i < points.size(); ++i)
      node_ptrs.push_back(mesh->add_point(points[i], i)); // Mesh owns the node

    std::vector<std::tuple<unsigned int, unsigned int, unsigned int>> closed_faces = {
        {0, 1, 2}, {0, 1, 3}, {1, 2, 3}, {0, 2, 3}};
    for (const auto & [id1, id2, id3] : closed_faces)
    {
      auto tri = new Tri3();
      tri->set_node(0) = node_ptrs[id1];
      tri->set_node(1) = node_ptrs[id2];
      tri->set_node(2) = node_ptrs[id3];
      mesh->add_elem(tri); // Mesh owns the element
    }

    mesh->prepare_for_use(false); // Neighbor info setup

    std::vector<const Elem *> raw_ptrs;
    for (const auto * el : mesh->active_local_element_ptr_range())
      raw_ptrs.push_back(el);

    EXPECT_TRUE(SBMUtils::checkWatertightnessFromRawElems(raw_ptrs));
  }
}
