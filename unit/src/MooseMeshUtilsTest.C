//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseMeshUtils.h"

#include "libmesh/boundary_info.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/replicated_mesh.h"

TEST(MooseMeshUtilsTest, getAllSubdomainNamesAndIDs)
{
  Parallel::Communicator comm;
  ReplicatedMesh mesh(comm);
  MeshTools::Generation::build_square(mesh, 2, 1);
  // Second element in a named subdomain, first one left in the unnamed subdomain 0
  mesh.elem_ref(1).subdomain_id() = 5;
  mesh.set_subdomain_name(5, "fuel");
  mesh.prepare_for_use();

  const std::set<std::pair<SubdomainName, SubdomainID>> expected = {{"", 0}, {"fuel", 5}};
  EXPECT_EQ(MooseMeshUtils::getAllSubdomainNamesAndIDs(mesh), expected);
}

TEST(MooseMeshUtilsTest, getAllBoundaryNamesAndIDs)
{
  Parallel::Communicator comm;
  ReplicatedMesh mesh(comm);
  MeshTools::Generation::build_square(mesh, 1, 1);
  auto & binfo = mesh.get_boundary_info();
  // A named nodeset without a sideset, and an unnamed nodeset
  binfo.add_node(mesh.node_ptr(0), 10);
  binfo.nodeset_name(10) = "corner";
  binfo.add_node(mesh.node_ptr(1), 11);

  const std::set<std::pair<BoundaryName, BoundaryID>> expected = {
      {"bottom", 0}, {"right", 1}, {"top", 2}, {"left", 3}, {"corner", 10}, {"", 11}};
  EXPECT_EQ(MooseMeshUtils::getAllBoundaryNamesAndIDs(mesh), expected);
}

TEST(MooseMeshUtilsTest, stitchMeshesPairwise)
{
  Parallel::Communicator comm;
  // build_square boundary ids: right = 1, left = 3. An odd count carries a mesh over to the next
  // level of the reduction
  for (const unsigned int n_meshes : {1, 2, 3, 4, 5})
  {
    std::vector<std::unique_ptr<ReplicatedMesh>> meshes;
    for (const auto m : make_range(n_meshes))
    {
      auto mesh = std::make_unique<ReplicatedMesh>(comm);
      MeshTools::Generation::build_square(*mesh, 2, 1);
      MeshTools::Modification::translate(*mesh, m, 0, 0);
      meshes.push_back(std::move(mesh));
    }

    const auto stitched = MooseMeshUtils::stitchMeshesPairwise(
        std::move(meshes),
        [](const MeshBase &) { return BoundaryID(1); },
        [](const MeshBase &) { return BoundaryID(3); },
        /*clear_stitched_boundary_ids=*/true,
        /*verbose=*/false);

    // 2 elements and 3 x 2 nodes per mesh, with a column of nodes shared by each stitched pair
    EXPECT_EQ(stitched->n_elem(), 2 * n_meshes);
    EXPECT_EQ(stitched->n_nodes(), 2 * (2 * n_meshes + 1));

    // The elements are in the order of the list
    Real previous_x = -1;
    for (const auto id : make_range(stitched->n_elem()))
    {
      const auto x = stitched->elem_ref(id).vertex_average()(0);
      EXPECT_GT(x, previous_x);
      previous_x = x;
    }
  }
}
