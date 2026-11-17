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
