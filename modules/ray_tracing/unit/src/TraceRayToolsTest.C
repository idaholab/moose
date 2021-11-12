//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "TraceRayTools.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/replicated_mesh.h"

std::unique_ptr<UnstructuredMesh>
traceRayToolsTestMesh(const int type)
{
  Parallel::Communicator comm;
  std::unique_ptr<UnstructuredMesh> mesh = std::make_unique<ReplicatedMesh>(comm);

  const unsigned int n = 2;
  const Real min = 0;
  const Real max = 1;
  switch ((ElemType)type)
  {
    case EDGE2:
    case EDGE3:
    case EDGE4:
      MeshTools::Generation::build_line(*mesh, n, min, max, (ElemType)type);
      break;
    case QUAD4:
    case QUAD8:
    case QUAD9:
    case TRI3:
    case TRI6:
      MeshTools::Generation::build_square(*mesh, n, n, min, max, min, max, (ElemType)type);
      break;
    case HEX8:
    case HEX20:
    case HEX27:
    case TET4:
    case TET10:
    case PYRAMID5:
    case PYRAMID13:
    case PYRAMID14:
    case PRISM6:
    case PRISM15:
    case PRISM18:
      MeshTools::Generation::build_cube(
          *mesh, n, n, n, min, max, min, max, min, max, (ElemType)type);
      break;
    default:
      break;
  }

  if (mesh)
    mesh->prepare_for_use();

  return mesh;
}

TEST(TraceRayToolsTest, withinEdge)
{
  ElemExtrema extrema;

  for (const auto type : TraceRayTools::TRACEABLE_ELEMTYPES)
  {
    auto mesh = traceRayToolsTestMesh(type);
    EXPECT_TRUE(mesh);

    if (mesh->mesh_dimension() != 3)
      continue;

    for (const auto elem : mesh->element_ptr_range())
    {
      for (const auto e : elem->edge_index_range())
      {
        extrema.invalidate();
        EXPECT_TRUE(TraceRayTools::withinEdge(elem, elem->build_edge_ptr(e)->vertex_average(), extrema));
        EXPECT_TRUE(extrema.atEdge(elem->nodes_on_edge(e)[0], elem->nodes_on_edge(e)[1]));
      }
      for (const auto n : elem->node_index_range())
        if (elem->is_vertex(n))
        {
          extrema.invalidate();
          EXPECT_TRUE(TraceRayTools::withinEdge(elem, elem->point(n), extrema));

          bool extrema_correct = false;
          for (const auto e : elem->edge_index_range())
            if (!extrema_correct && elem->is_node_on_edge(n, e))
              extrema_correct =
                  extrema.atEdge(elem->nodes_on_edge(e)[0], elem->nodes_on_edge(e)[1]);
          EXPECT_TRUE(extrema_correct);
        }
    }
  }
}

TEST(TraceRayToolsTest, withinEdgeOnSide)
{
  ElemExtrema extrema;

  for (const auto type : TraceRayTools::TRACEABLE_ELEMTYPES)
  {
    auto mesh = traceRayToolsTestMesh(type);
    EXPECT_TRUE(mesh);

    if (mesh->mesh_dimension() != 3)
      continue;

    for (const auto elem : mesh->element_ptr_range())
    {
      for (const auto s : elem->side_index_range())
      {
        extrema.invalidate();
        EXPECT_FALSE(
            TraceRayTools::withinEdgeOnSide(elem, elem->build_side_ptr(s)->vertex_average(), s, extrema));
        EXPECT_TRUE(extrema.isInvalid());

        for (const auto e : elem->edge_index_range())
          if (elem->is_edge_on_side(e, s))
          {
            extrema.invalidate();
            EXPECT_TRUE(TraceRayTools::withinEdgeOnSide(
                elem, elem->build_edge_ptr(e)->vertex_average(), s, extrema));
          }
      }
      for (const auto n : elem->node_index_range())
        if (elem->is_vertex(n))
          for (const auto s : elem->side_index_range())
            if (elem->is_node_on_side(n, s))
            {
              extrema.invalidate();
              EXPECT_TRUE(TraceRayTools::withinEdgeOnSide(elem, elem->point(n), s, extrema));

              bool extrema_correct = false;
              for (const auto e : elem->edge_index_range())
                if (!extrema_correct && elem->is_node_on_edge(n, e))
                  extrema_correct =
                      extrema.atEdge(elem->nodes_on_edge(e)[0], elem->nodes_on_edge(e)[1]);
              EXPECT_TRUE(extrema_correct);
            }
    }
  }
}

TEST(TraceRayToolsTest, atVertex)
{
  for (const auto type : TraceRayTools::TRACEABLE_ELEMTYPES)
  {
    auto mesh = traceRayToolsTestMesh(type);
    EXPECT_TRUE(mesh);

    for (const auto elem : mesh->element_ptr_range())
    {
      EXPECT_EQ(TraceRayTools::atVertex(elem, elem->vertex_average()), RayTracingCommon::invalid_vertex);
      for (const auto n : elem->node_index_range())
        if (elem->is_vertex(n))
        {
          EXPECT_EQ((unsigned short)n, TraceRayTools::atVertex(elem, elem->point(n)));
        }
    }
  }
}

TEST(TraceRayToolsTest, atVertexOnSide)
{
  for (const auto type : TraceRayTools::TRACEABLE_ELEMTYPES)
  {
    auto mesh = traceRayToolsTestMesh(type);
    EXPECT_TRUE(mesh);

    for (const auto elem : mesh->element_ptr_range())
      for (const auto s : elem->side_index_range())
      {
        if (elem->dim() > 1)
        {
          EXPECT_EQ(TraceRayTools::atVertexOnSide(elem, elem->build_side_ptr(s)->vertex_average(), s),
                    RayTracingCommon::invalid_vertex);
        }

        for (const auto n : elem->nodes_on_side(s))
          if (elem->is_vertex(n))
          {
            EXPECT_EQ(TraceRayTools::atVertexOnSide(elem, elem->point(n), s), (unsigned short)n);
          }
      }
  }
}

TEST(TraceRayToolsTest, findPointNeighbors)
{
  MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> neighbor_set;
  MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> neighbor_untested_set;
  MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> neighbor_next_untested_set;
  std::vector<const Elem *> active_neighbor_children;
  std::vector<NeighborInfo> neighbor_info;
  std::set<const Elem *> libmesh_neighbor_set;

  for (const auto type : TraceRayTools::TRACEABLE_ELEMTYPES)
  {
    auto mesh = traceRayToolsTestMesh(type);
    EXPECT_TRUE(mesh);

    for (const auto elem : mesh->element_ptr_range())
    {
      for (const auto n : elem->node_index_range())
      {
        TraceRayTools::findPointNeighbors(elem,
                                          elem->point(n),
                                          neighbor_set,
                                          neighbor_untested_set,
                                          neighbor_next_untested_set,
                                          active_neighbor_children,
                                          neighbor_info);
        elem->find_point_neighbors(elem->point(n), libmesh_neighbor_set);
        for (const auto & info : neighbor_info)
        {
          for (const auto s : info._sides)
            EXPECT_TRUE(info._elem->build_side_ptr(s)->contains_point(elem->point(n)));
          EXPECT_TRUE(libmesh_neighbor_set.count(info._elem));
        }
      }

      for (const auto s : elem->side_index_range())
      {
        const auto centroid = elem->build_side_ptr(s)->vertex_average();
        TraceRayTools::findPointNeighbors(elem,
                                          centroid,
                                          neighbor_set,
                                          neighbor_untested_set,
                                          neighbor_next_untested_set,
                                          active_neighbor_children,
                                          neighbor_info);
        elem->find_point_neighbors(centroid, libmesh_neighbor_set);
        for (const auto & info : neighbor_info)
        {
          for (const auto other_s : info._sides)
            EXPECT_TRUE(info._elem->build_side_ptr(other_s)->contains_point(centroid));
          EXPECT_TRUE(libmesh_neighbor_set.count(info._elem));
        }
      }

      if (elem->dim() == 3)
        for (const auto e : elem->edge_index_range())
        {
          const auto centroid = elem->build_edge_ptr(e)->vertex_average();
          TraceRayTools::findPointNeighbors(elem,
                                            centroid,
                                            neighbor_set,
                                            neighbor_untested_set,
                                            neighbor_next_untested_set,
                                            active_neighbor_children,
                                            neighbor_info);
          elem->find_point_neighbors(centroid, libmesh_neighbor_set);
          for (const auto & info : neighbor_info)
          {
            for (const auto s : info._sides)
              EXPECT_TRUE(info._elem->build_side_ptr(s)->contains_point(centroid));
            EXPECT_TRUE(libmesh_neighbor_set.count(info._elem));
          }
        }
    }
  }
}

TEST(TraceRayToolsTest, isWithinSegment)
{
  EXPECT_TRUE(TraceRayTools::isWithinSegment(Point(0, 1, 2), Point(1, 1, 2), Point(0.5, 1, 2)));
  EXPECT_TRUE(TraceRayTools::isWithinSegment(Point(5, 6, 7), Point(8, 9, 10), Point(5, 6, 7)));
  EXPECT_TRUE(TraceRayTools::isWithinSegment(Point(2, 3, 4), Point(4, 5, 6), Point(4, 5, 6)));
  EXPECT_FALSE(TraceRayTools::isWithinSegment(Point(1, 2, 3), Point(4, 5, 6), Point(10, 0, 0)));
}
