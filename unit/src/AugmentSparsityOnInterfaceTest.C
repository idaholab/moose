//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "AugmentSparsityOnInterface.h"
#include "Factory.h"
#include "MeshGeneratorMesh.h"
#include "Moose.h"
#include "MooseApp.h"
#include "MooseMain.h"
#include "MooseUnitApp.h"
#include "RelationshipManager.h"

#include "libmesh/boundary_info.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/face_quad4.h"
#include "libmesh/mesh_base.h"
#include "libmesh/point.h"

#include <array>

using namespace libMesh;

namespace
{
constexpr BoundaryID primary_boundary_id = 20;
constexpr BoundaryID secondary_boundary_id = 21;
constexpr SubdomainID primary_subdomain_id = 120;
constexpr SubdomainID secondary_subdomain_id = 121;
const BoundaryName primary_boundary_name = "mortar_primary";
const BoundaryName secondary_boundary_name = "mortar_secondary";
const SubdomainName primary_subdomain_name = "mortar_primary_lower";
const SubdomainName secondary_subdomain_name = "mortar_secondary_lower";

constexpr unique_id_type elem_unique_id_offset = 2000;

constexpr dof_id_type local_secondary_elem_id = 0;
constexpr dof_id_type remote_primary_elem_id = 1;
constexpr dof_id_type remote_secondary_elem_id = 2;
constexpr dof_id_type remote_noninterface_elem_id = 3;
constexpr dof_id_type local_secondary_lower_elem_id = 10;
constexpr dof_id_type remote_primary_lower_elem_id = 11;
constexpr dof_id_type remote_secondary_lower_elem_id = 12;

constexpr dof_id_type owner_secondary_elem_id = 20;
constexpr dof_id_type owner_primary_elem_id = 21;
constexpr dof_id_type non_owner_primary_elem_id = 22;
constexpr dof_id_type non_owner_noninterface_elem_id = 23;
constexpr dof_id_type owner_secondary_lower_elem_id = 30;
constexpr dof_id_type owner_primary_lower_elem_id = 31;
constexpr dof_id_type non_owner_primary_lower_elem_id = 32;

Elem *
addDisconnectedQuad(MeshBase & mesh,
                    const dof_id_type elem_id,
                    const processor_id_type processor_id,
                    const Real x_offset)
{
  std::array<Node *, 4> nodes;
  const std::array<Point, 4> points = {Point(x_offset, 0, 0),
                                       Point(x_offset + 1, 0, 0),
                                       Point(x_offset + 1, 1, 0),
                                       Point(x_offset, 1, 0)};
  const auto first_node_id = 4 * elem_id;

  for (const auto i : make_range(nodes.size()))
  {
    nodes[i] = mesh.add_point(points[i], first_node_id + i, processor_id);
    nodes[i]->processor_id() = processor_id;
#ifdef LIBMESH_ENABLE_UNIQUE_ID
    nodes[i]->set_unique_id(first_node_id + i);
#endif
  }

  auto elem = std::make_unique<Quad4>();
  elem->set_id(elem_id);
  elem->processor_id() = processor_id;
  elem->subdomain_id() = 0;
#ifdef LIBMESH_ENABLE_UNIQUE_ID
  elem->set_unique_id(elem_unique_id_offset + elem_id);
#endif

  for (const auto i : make_range(nodes.size()))
    elem->set_node(i, nodes[i]);

  return mesh.insert_elem(std::move(elem));
}

Elem *
addLowerDEdge(MeshBase & mesh,
              Elem & interior_parent,
              const unsigned int side,
              const dof_id_type elem_id,
              const SubdomainID subdomain_id)
{
  auto elem = interior_parent.build_side_ptr(side);
  elem->set_id(elem_id);
  elem->processor_id() = interior_parent.processor_id();
  elem->subdomain_id() = subdomain_id;
  elem->set_interior_parent(&interior_parent);
#ifdef LIBMESH_ENABLE_UNIQUE_ID
  elem->set_unique_id(elem_unique_id_offset + elem_id);
#endif

  return mesh.add_elem(elem.release());
}

void
setupMesh(MeshBase & mesh)
{
  mesh.set_mesh_dimension(2);
  mesh.allow_renumbering(false);
  mesh.skip_partitioning(true);
  mesh.allow_remote_element_removal(true);

  auto & boundary_info = mesh.get_boundary_info();
  boundary_info.sideset_name(primary_boundary_id) = primary_boundary_name;
  boundary_info.nodeset_name(primary_boundary_id) = primary_boundary_name;
  boundary_info.sideset_name(secondary_boundary_id) = secondary_boundary_name;
  boundary_info.nodeset_name(secondary_boundary_id) = secondary_boundary_name;
  mesh.subdomain_name(primary_subdomain_id) = primary_subdomain_name;
  mesh.subdomain_name(secondary_subdomain_id) = secondary_subdomain_name;
}

void
addPrimaryInterface(MeshBase & mesh,
                    const dof_id_type elem_id,
                    const dof_id_type lower_elem_id,
                    const processor_id_type processor_id,
                    const Real x_offset)
{
  auto * const elem = addDisconnectedQuad(mesh, elem_id, processor_id, x_offset);
  mesh.get_boundary_info().add_side(elem, 0, primary_boundary_id);
  addLowerDEdge(mesh, *elem, 0, lower_elem_id, primary_subdomain_id);
}

void
addSecondaryInterface(MeshBase & mesh,
                      const dof_id_type elem_id,
                      const dof_id_type lower_elem_id,
                      const processor_id_type processor_id,
                      const Real x_offset)
{
  auto * const elem = addDisconnectedQuad(mesh, elem_id, processor_id, x_offset);
  mesh.get_boundary_info().add_side(elem, 0, secondary_boundary_id);
  addLowerDEdge(mesh, *elem, 0, lower_elem_id, secondary_subdomain_id);
}

void
buildPrimaryOnlyGhostingMesh(MeshBase & mesh)
{
  setupMesh(mesh);

  const processor_id_type rank0 = 0;
  const processor_id_type rank1 = 1;

  addSecondaryInterface(mesh, local_secondary_elem_id, local_secondary_lower_elem_id, rank0, 0);
  addPrimaryInterface(mesh, remote_primary_elem_id, remote_primary_lower_elem_id, rank1, 3);
  addSecondaryInterface(mesh, remote_secondary_elem_id, remote_secondary_lower_elem_id, rank1, 6);
  addDisconnectedQuad(mesh, remote_noninterface_elem_id, rank1, 9);
}

void
buildSecondaryOwnerGhostingMesh(MeshBase & mesh)
{
  setupMesh(mesh);

  const processor_id_type rank0 = 0;
  const processor_id_type rank1 = 1;

  addSecondaryInterface(mesh, owner_secondary_elem_id, owner_secondary_lower_elem_id, rank0, 0);
  addPrimaryInterface(mesh, owner_primary_elem_id, owner_primary_lower_elem_id, rank0, 3);
  addPrimaryInterface(mesh, non_owner_primary_elem_id, non_owner_primary_lower_elem_id, rank1, 6);
  addDisconnectedQuad(mesh, non_owner_noninterface_elem_id, rank1, 9);
}
}

class AugmentSparsityOnInterfaceTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    const char * argv[2] = {"foo", "\0"};
    _app = Moose::createMooseApp("MooseUnitApp", 1, (char **)argv);
    _factory = &_app->getFactory();
  }

  std::shared_ptr<MeshGeneratorMesh> buildMooseMesh()
  {
    auto params = _factory->getValidParams("MeshGeneratorMesh");
    params.set<MooseEnum>("parallel_type") = "DISTRIBUTED";
    params.set<bool>("allow_renumbering") = false;

    auto moose_mesh =
        _factory->create<MeshGeneratorMesh>("MeshGeneratorMesh", "moose_mesh", params);
    moose_mesh->setMeshBase(moose_mesh->buildMeshBaseObject(2));
    moose_mesh->buildMesh();
    _app->actionWarehouse().mesh() = moose_mesh;
    return moose_mesh;
  }

  std::shared_ptr<RelationshipManager> createAndAttachRelationshipManager(MooseMesh & moose_mesh,
                                                                          MeshBase & mesh)
  {
    auto rm_params = _factory->getValidParams("AugmentSparsityOnInterface");
    rm_params.set<Moose::RelationshipManagerType>("rm_type") =
        Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC;
    rm_params.set<std::string>("for_whom") = "AugmentSparsityOnInterfaceTest";
    rm_params.set<MooseMesh *>("mesh") = &moose_mesh;
    rm_params.set<bool>("use_displaced_mesh") = true;
    rm_params.set<BoundaryName>("primary_boundary") = primary_boundary_name;
    rm_params.set<BoundaryName>("secondary_boundary") = secondary_boundary_name;
    rm_params.set<SubdomainName>("primary_subdomain") = primary_subdomain_name;
    rm_params.set<SubdomainName>("secondary_subdomain") = secondary_subdomain_name;

    auto rm = _factory->create<RelationshipManager>(
        "AugmentSparsityOnInterface", "augment_sparsity_on_interface", rm_params);
    rm->init(moose_mesh, mesh);
    mesh.add_ghosting_functor(*rm);
    return rm;
  }

  std::shared_ptr<MooseApp> _app;
  Factory * _factory;
};

TEST_F(AugmentSparsityOnInterfaceTest, displacedWholeInterfaceGhostsOnlyPrimaryInterface)
{
  if (_app->n_processors() != 2)
    GTEST_SKIP() << "This test requires exactly two MPI ranks.";

  auto moose_mesh = buildMooseMesh();
  auto & mesh = moose_mesh->getMesh();
  buildPrimaryOnlyGhostingMesh(mesh);
  const auto rm = createAndAttachRelationshipManager(*moose_mesh, mesh);
  ASSERT_TRUE(rm);

  mesh.complete_preparation();

  if (mesh.processor_id() == 0)
  {
    EXPECT_NE(mesh.query_elem_ptr(local_secondary_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(local_secondary_lower_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(remote_primary_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(remote_primary_lower_elem_id), nullptr);
    EXPECT_EQ(mesh.query_elem_ptr(remote_secondary_elem_id), nullptr);
    EXPECT_EQ(mesh.query_elem_ptr(remote_secondary_lower_elem_id), nullptr);
    EXPECT_EQ(mesh.query_elem_ptr(remote_noninterface_elem_id), nullptr);
  }
  else if (mesh.processor_id() == 1)
  {
    EXPECT_EQ(mesh.query_elem_ptr(local_secondary_elem_id), nullptr);
    EXPECT_EQ(mesh.query_elem_ptr(local_secondary_lower_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(remote_primary_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(remote_primary_lower_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(remote_secondary_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(remote_secondary_lower_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(remote_noninterface_elem_id), nullptr);
  }
}

TEST_F(AugmentSparsityOnInterfaceTest, displacedWholeInterfaceGhostsOnlyOnSecondaryOwners)
{
  if (_app->n_processors() != 2)
    GTEST_SKIP() << "This test requires exactly two MPI ranks.";

  auto moose_mesh = buildMooseMesh();
  auto & mesh = moose_mesh->getMesh();
  buildSecondaryOwnerGhostingMesh(mesh);
  const auto rm = createAndAttachRelationshipManager(*moose_mesh, mesh);
  ASSERT_TRUE(rm);

  mesh.complete_preparation();

  if (mesh.processor_id() == 0)
  {
    EXPECT_NE(mesh.query_elem_ptr(owner_secondary_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(owner_secondary_lower_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(owner_primary_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(owner_primary_lower_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(non_owner_primary_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(non_owner_primary_lower_elem_id), nullptr);
    EXPECT_EQ(mesh.query_elem_ptr(non_owner_noninterface_elem_id), nullptr);
  }
  else if (mesh.processor_id() == 1)
  {
    EXPECT_EQ(mesh.query_elem_ptr(owner_secondary_elem_id), nullptr);
    EXPECT_EQ(mesh.query_elem_ptr(owner_secondary_lower_elem_id), nullptr);
    EXPECT_EQ(mesh.query_elem_ptr(owner_primary_elem_id), nullptr);
    EXPECT_EQ(mesh.query_elem_ptr(owner_primary_lower_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(non_owner_primary_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(non_owner_primary_lower_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(non_owner_noninterface_elem_id), nullptr);
  }
}
