//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "Action.h"
#include "ActionFactory.h"
#include "Factory.h"
#include "GhostNodeFaceInterface.h"
#include "MeshGeneratorMesh.h"
#include "Moose.h"
#include "MooseApp.h"
#include "MooseMain.h"
#include "MooseUnitApp.h"
#include "Registry.h"
#include "RelationshipManager.h"
#include "TiedValueConstraint.h"

#include "libmesh/boundary_info.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/face_quad4.h"
#include "libmesh/mesh_base.h"
#include "libmesh/point.h"

#include <array>
#include <memory>

using namespace libMesh;

namespace
{
constexpr BoundaryID primary_boundary_id = 10;
constexpr BoundaryID secondary_boundary_id = 11;
const BoundaryName primary_boundary_name = "primary";
const BoundaryName secondary_boundary_name = "secondary";

constexpr dof_id_type local_secondary_elem_id = 0;
constexpr dof_id_type remote_secondary_elem_id = 1;
constexpr dof_id_type remote_primary_elem_id = 2;
constexpr dof_id_type remote_noninterface_elem_id = 3;
constexpr unique_id_type elem_unique_id_offset = 1000;

class TestNodeFaceRelationshipManagerAction : public Action
{
public:
  static InputParameters validParams() { return Action::validParams(); }

  TestNodeFaceRelationshipManagerAction(const InputParameters & params) : Action(params) {}

  void act() override {}

  bool addRelationshipManagersFrom(const InputParameters & object_params,
                                   Moose::RelationshipManagerType rm_type)
  {
    return addRelationshipManagers(rm_type, object_params);
  }
};

registerMooseAction("MooseUnitApp",
                    TestNodeFaceRelationshipManagerAction,
                    "add_test_node_face_rms");

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

void
buildNodeFaceInterfaceMesh(MeshBase & mesh)
{
  const processor_id_type rank0 = 0;
  const processor_id_type rank1 = 1;

  mesh.set_mesh_dimension(2);
  mesh.allow_renumbering(false);
  mesh.skip_partitioning(true);
  mesh.allow_remote_element_removal(true);

  auto * const local_secondary = addDisconnectedQuad(mesh, local_secondary_elem_id, rank0, 0);
  auto * const remote_secondary = addDisconnectedQuad(mesh, remote_secondary_elem_id, rank1, 3);
  auto * const remote_primary = addDisconnectedQuad(mesh, remote_primary_elem_id, rank1, 6);
  addDisconnectedQuad(mesh, remote_noninterface_elem_id, rank1, 9);

  auto & boundary_info = mesh.get_boundary_info();
  boundary_info.sideset_name(primary_boundary_id) = primary_boundary_name;
  boundary_info.nodeset_name(primary_boundary_id) = primary_boundary_name;
  boundary_info.sideset_name(secondary_boundary_id) = secondary_boundary_name;
  boundary_info.nodeset_name(secondary_boundary_id) = secondary_boundary_name;

  boundary_info.add_side(local_secondary, 0, secondary_boundary_id);
  boundary_info.add_side(remote_secondary, 0, secondary_boundary_id);
  boundary_info.add_side(remote_primary, 0, primary_boundary_id);
}
}

class GhostNodeFaceInterfaceTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    const char * argv[2] = {"foo", "\0"};
    _app = Moose::createMooseApp("MooseUnitApp", 1, (char **)argv);
    _factory = &_app->getFactory();
    Registry::registerActionsTo(_app->getActionFactory(), {"MooseUnitApp"});
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

  std::shared_ptr<RelationshipManager>
  createAndAttachRelationshipManager(MooseMesh & moose_mesh, MeshBase & mesh, const bool enabled)
  {
    auto rm_params = _factory->getValidParams("GhostNodeFaceInterface");
    rm_params.set<Moose::RelationshipManagerType>("rm_type") =
        Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC;
    rm_params.set<std::string>("for_whom") = "GhostNodeFaceInterfaceTest";
    rm_params.set<MooseMesh *>("mesh") = &moose_mesh;
    rm_params.set<bool>("use_displaced_mesh") = false;
    rm_params.set<bool>("enabled") = enabled;
    rm_params.set<BoundaryName>("primary_boundary") = primary_boundary_name;
    rm_params.set<BoundaryName>("secondary_boundary") = secondary_boundary_name;

    auto rm = _factory->create<RelationshipManager>(
        "GhostNodeFaceInterface", "ghost_node_face", rm_params);
    rm->init(moose_mesh, mesh);
    mesh.add_ghosting_functor(*rm);
    return rm;
  }

  InputParameters tiedValueParams(const bool set_ghost_whole_interface)
  {
    auto params = _factory->getValidParams("TiedValueConstraint");
    params.set<BoundaryName>("primary") = primary_boundary_name;
    params.set<BoundaryName>("secondary") = secondary_boundary_name;
    if (set_ghost_whole_interface)
      params.set<bool>("ghost_whole_interface") = true;
    return params;
  }

  std::shared_ptr<TestNodeFaceRelationshipManagerAction> buildTestAction()
  {
    auto params = _app->getActionFactory().getValidParams("TestNodeFaceRelationshipManagerAction");
    params.set<std::string>("task") = "add_test_node_face_rms";
    params.set<std::string>("registered_identifier") = "test_node_face_rm";
    auto action = _app->getActionFactory().create(
        "TestNodeFaceRelationshipManagerAction", "test_node_face_rm", params);
    return std::dynamic_pointer_cast<TestNodeFaceRelationshipManagerAction>(action);
  }

  std::shared_ptr<MooseApp> _app;
  Factory * _factory;
};

TEST_F(GhostNodeFaceInterfaceTest, enabledRMKeepsRemotePrimaryInterfaceElements)
{
  if (_app->n_processors() != 2)
    GTEST_SKIP() << "This test requires exactly two MPI ranks.";

  auto moose_mesh = buildMooseMesh();
  auto & mesh = moose_mesh->getMesh();
  buildNodeFaceInterfaceMesh(mesh);
  const auto rm = createAndAttachRelationshipManager(*moose_mesh, mesh, true);
  ASSERT_TRUE(rm);

  mesh.complete_preparation();

  if (mesh.processor_id() == 0)
  {
    EXPECT_NE(mesh.query_elem_ptr(local_secondary_elem_id), nullptr);
    EXPECT_EQ(mesh.query_elem_ptr(remote_secondary_elem_id), nullptr);
    EXPECT_NE(mesh.query_elem_ptr(remote_primary_elem_id), nullptr);
    EXPECT_EQ(mesh.query_elem_ptr(remote_noninterface_elem_id), nullptr);
  }
}

TEST_F(GhostNodeFaceInterfaceTest, disabledRMAllowsRemoteInterfaceElementsToBeRemoved)
{
  if (_app->n_processors() != 2)
    GTEST_SKIP() << "This test requires exactly two MPI ranks.";

  auto moose_mesh = buildMooseMesh();
  auto & mesh = moose_mesh->getMesh();
  buildNodeFaceInterfaceMesh(mesh);
  const auto rm = createAndAttachRelationshipManager(*moose_mesh, mesh, false);
  ASSERT_TRUE(rm);

  mesh.complete_preparation();

  if (mesh.processor_id() == 0)
  {
    EXPECT_NE(mesh.query_elem_ptr(local_secondary_elem_id), nullptr);
    EXPECT_EQ(mesh.query_elem_ptr(remote_secondary_elem_id), nullptr);
    EXPECT_EQ(mesh.query_elem_ptr(remote_primary_elem_id), nullptr);
    EXPECT_EQ(mesh.query_elem_ptr(remote_noninterface_elem_id), nullptr);
  }
}

TEST_F(GhostNodeFaceInterfaceTest, nodeFaceDerivedConstraintAddsDisabledRMByDefault)
{
  buildMooseMesh();
  const auto action = buildTestAction();
  ASSERT_TRUE(action);

  ASSERT_TRUE(action->addRelationshipManagersFrom(tiedValueParams(false),
                                                  Moose::RelationshipManagerType::GEOMETRIC));

  const auto & relationship_managers = _app->relationshipManagers();
  ASSERT_EQ(relationship_managers.size(), 1);

  auto & rm = **relationship_managers.begin();
  EXPECT_EQ(rm.type(), "GhostNodeFaceInterface");
  EXPECT_TRUE(rm.isType(Moose::RelationshipManagerType::GEOMETRIC));
  EXPECT_TRUE(rm.isType(Moose::RelationshipManagerType::ALGEBRAIC));
  EXPECT_TRUE(rm.attachGeometricEarly());
  EXPECT_EQ(rm.getInfo(), "GhostNodeFaceInterface (disabled)");
}

TEST_F(GhostNodeFaceInterfaceTest, nodeFaceDerivedConstraintCanEnableWholeInterfaceRM)
{
  buildMooseMesh();
  const auto action = buildTestAction();
  ASSERT_TRUE(action);

  ASSERT_TRUE(action->addRelationshipManagersFrom(tiedValueParams(true),
                                                  Moose::RelationshipManagerType::GEOMETRIC));

  const auto & relationship_managers = _app->relationshipManagers();
  ASSERT_EQ(relationship_managers.size(), 1);

  auto & rm = **relationship_managers.begin();
  EXPECT_EQ(rm.type(), "GhostNodeFaceInterface");
  EXPECT_TRUE(rm.isType(Moose::RelationshipManagerType::GEOMETRIC));
  EXPECT_TRUE(rm.isType(Moose::RelationshipManagerType::ALGEBRAIC));
  EXPECT_FALSE(rm.attachGeometricEarly());
  EXPECT_EQ(rm.getInfo(), "GhostNodeFaceInterface");
}
