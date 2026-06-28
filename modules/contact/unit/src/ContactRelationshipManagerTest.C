//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "ActionFactory.h"
#include "ContactAction.h"
#include "Factory.h"
#include "MeshGeneratorMesh.h"
#include "Moose.h"
#include "MooseApp.h"
#include "MooseMain.h"
#include "RelationshipManager.h"

class ContactRelationshipManagerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    const char * argv[2] = {"foo", "\0"};
    _app = Moose::createMooseApp("ContactApp", 1, (char **)argv);
    _factory = &_app->getFactory();
    buildMooseMesh();
  }

  void buildMooseMesh()
  {
    auto params = _factory->getValidParams("MeshGeneratorMesh");
    auto moose_mesh =
        _factory->create<MeshGeneratorMesh>("MeshGeneratorMesh", "moose_mesh", params);
    moose_mesh->setMeshBase(moose_mesh->buildMeshBaseObject(2));
    moose_mesh->buildMesh();
    _app->actionWarehouse().mesh() = moose_mesh;
  }

  std::shared_ptr<ContactAction> buildContactAction(const bool set_ghost_whole_interface)
  {
    auto params = _app->getActionFactory().getValidParams("ContactAction");
    params.set<std::string>("task") = "add_constraint";
    params.set<std::string>("registered_identifier") = "Contact/*";
    params.set<std::vector<BoundaryName>>("primary") = {"primary"};
    params.set<std::vector<BoundaryName>>("secondary") = {"secondary"};
    if (set_ghost_whole_interface)
      params.set<bool>("ghost_whole_interface") = true;

    auto action = _app->getActionFactory().create("ContactAction", "Contact/test", params);
    return std::dynamic_pointer_cast<ContactAction>(action);
  }

  void testContactActionRelationshipManager(const bool enabled)
  {
    const auto action = buildContactAction(enabled);
    ASSERT_TRUE(action);

    action->addRelationshipManagers(Moose::RelationshipManagerType::GEOMETRIC);

    const auto & relationship_managers = _app->relationshipManagers();
    ASSERT_EQ(relationship_managers.size(), 1);

    auto & rm = **relationship_managers.begin();
    EXPECT_EQ(rm.type(), "GhostNodeFaceInterface");
    EXPECT_TRUE(rm.isType(Moose::RelationshipManagerType::GEOMETRIC));
    EXPECT_TRUE(rm.isType(Moose::RelationshipManagerType::ALGEBRAIC));
    EXPECT_EQ(rm.attachGeometricEarly(), !enabled);
    EXPECT_EQ(rm.getInfo(),
              enabled ? "GhostNodeFaceInterface" : "GhostNodeFaceInterface (disabled)");
  }

  std::shared_ptr<MooseApp> _app;
  Factory * _factory;
};

TEST_F(ContactRelationshipManagerTest, contactActionAddsDisabledRMByDefault)
{
  testContactActionRelationshipManager(false);
}

TEST_F(ContactRelationshipManagerTest, contactActionCanEnableWholeInterfaceRM)
{
  testContactActionRelationshipManager(true);
}
