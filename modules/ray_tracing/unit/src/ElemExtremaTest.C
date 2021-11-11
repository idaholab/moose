//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "ElemExtrema.h"
#include "TraceRayTools.h"

#include "libmesh/elem.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/replicated_mesh.h"

TEST(ElemExtremaTest, Test)
{
  Parallel::Communicator comm;
  std::unique_ptr<UnstructuredMesh> mesh = std::make_unique<ReplicatedMesh>(comm);
  MeshTools::Generation::build_cube(*mesh, 2, 2, 2);

  const Elem * elem = mesh->query_elem_ptr(0);
  EXPECT_TRUE(elem != nullptr);

  {
    ElemExtrema invalid;

    EXPECT_FALSE(invalid.atExtrema());
    EXPECT_TRUE(invalid.isInvalid());
    EXPECT_FALSE(invalid.atVertex());
    EXPECT_FALSE(invalid.atVertex(0));
    EXPECT_FALSE(invalid.atEdge());
    EXPECT_FALSE(invalid.atEdge(0, 1));
    EXPECT_TRUE(invalid.isValid(elem, elem->vertex_average()));

    std::stringstream oss;
    oss << invalid;
    EXPECT_EQ(oss.str(), "not at extrema");
  }

  {
    ElemExtrema at_vertex;
    at_vertex.setVertex(0);

    EXPECT_TRUE(at_vertex.atExtrema());
    EXPECT_FALSE(at_vertex.isInvalid());
    EXPECT_TRUE(at_vertex.atVertex());
    EXPECT_TRUE(at_vertex.atVertex(0));
    EXPECT_FALSE(at_vertex.atVertex(1));
    EXPECT_FALSE(at_vertex.atEdge());
    EXPECT_FALSE(at_vertex.atEdge(0, 1));
    EXPECT_EQ((unsigned short)0, at_vertex.vertex());

    const auto & vertex_point = at_vertex.vertexPoint(elem);
    const auto & point = elem->point(0);
    for (unsigned int d = 0; d < 3; ++d)
      EXPECT_NEAR(vertex_point(d), point(d), 1.e-10);

    std::stringstream oss;
    oss << at_vertex;
    EXPECT_EQ(oss.str(), "at vertex 0");

    EXPECT_TRUE(at_vertex.isValid(elem, elem->point(0)));

    at_vertex.invalidate();
    EXPECT_TRUE(at_vertex.isInvalid());
  }

  {
    Point edge_point(0.25, 0, 0);
    ElemExtrema at_edge_temp;

    EXPECT_TRUE(TraceRayTools::withinEdge(elem, edge_point, at_edge_temp));

    ElemExtrema at_edge;
    at_edge.setEdge(at_edge_temp.edgeVertices());

    EXPECT_TRUE(at_edge.atExtrema());
    EXPECT_FALSE(at_edge.isInvalid());
    EXPECT_FALSE(at_edge.atVertex());
    EXPECT_TRUE(at_edge.atEdge());
    EXPECT_TRUE(at_edge.atEdge(at_edge.edgeVertices().first, at_edge.edgeVertices().second));
    EXPECT_TRUE(at_edge.atEdge(at_edge.edgeVertices().second, at_edge.edgeVertices().first));

    const auto edge = at_edge.buildEdge(elem);
    for (unsigned int d = 0; d < 3; ++d)
    {
      EXPECT_NEAR(edge->point(0)(d), elem->point(at_edge.edgeVertices().first)(d), 1.e-10);
      EXPECT_NEAR(edge->point(1)(d), elem->point(at_edge.edgeVertices().second)(d), 1.e-10);
    }

    EXPECT_TRUE(at_edge.isValid(elem, edge_point));
    std::stringstream should_be;
    should_be << "at edge with vertices " << at_edge.edgeVertices().first << " and "
              << at_edge.edgeVertices().second;
    std::stringstream oss;
    oss << at_edge;
    EXPECT_EQ(oss.str(), should_be.str());
  }

  {
    ElemExtrema at_edge(0, 1);
    EXPECT_TRUE(at_edge.first == 0);
    EXPECT_TRUE(at_edge.second == 1);
  }
}
