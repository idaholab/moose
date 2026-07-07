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
#include "MooseUnitUtils.h"
#include "RelationshipManager.h"

class ContactRelationshipManagerTest : public ::testing::Test
{
protected:
  enum class GhostWholeInterfaceInput
  {
    Default,
    False,
    True
  };

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

  std::shared_ptr<ContactAction> buildContactAction(
      const GhostWholeInterfaceInput ghost_whole_interface = GhostWholeInterfaceInput::Default,
      const std::string & formulation = "kinematic")
  {
    auto params = _app->getActionFactory().getValidParams("ContactAction");
    params.set<std::string>("task") = "add_constraint";
    params.set<std::string>("registered_identifier") = "Contact/*";
    params.set<std::vector<BoundaryName>>("primary") = {"primary"};
    params.set<std::vector<BoundaryName>>("secondary") = {"secondary"};
    // Mark this programmatic assignment as user-set so the action sees the
    // same parameter metadata it would get from parsed input.
    params.set_attributes("formulation", false);
    params.set<MooseEnum>("formulation") = formulation;
    // The mortar validation specifically checks isParamSetByUser(), so only
    // explicit test inputs should be marked as user-set.
    switch (ghost_whole_interface)
    {
      case GhostWholeInterfaceInput::Default:
        break;
      case GhostWholeInterfaceInput::False:
        params.set_attributes("ghost_whole_interface", false);
        params.set<bool>("ghost_whole_interface") = false;
        break;
      case GhostWholeInterfaceInput::True:
        params.set_attributes("ghost_whole_interface", false);
        params.set<bool>("ghost_whole_interface") = true;
        break;
    }

    auto action = _app->getActionFactory().create("ContactAction", "Contact/test", params);
    return std::dynamic_pointer_cast<ContactAction>(action);
  }

  void testContactActionRelationshipManager(const bool enabled)
  {
    const auto action = buildContactAction(enabled ? GhostWholeInterfaceInput::True
                                                   : GhostWholeInterfaceInput::Default);
    ASSERT_TRUE(action);

    action->addRelationshipManagers(Moose::RelationshipManagerType::GEOMETRIC);

    const auto & relationship_managers = _app->relationshipManagers();
    ASSERT_EQ(relationship_managers.size(), 1);

    auto & rm = **relationship_managers.begin();
    EXPECT_EQ(rm.type(), "GhostPrimaryFace");
    EXPECT_TRUE(rm.isType(Moose::RelationshipManagerType::GEOMETRIC));
    EXPECT_TRUE(rm.isType(Moose::RelationshipManagerType::ALGEBRAIC));
    EXPECT_EQ(rm.attachGeometricEarly(), !enabled);
    EXPECT_EQ(rm.getInfo(), enabled ? "GhostPrimaryFace" : "GhostPrimaryFace (disabled)");
  }

  void testMortarContactActionRejectsGhostWholeInterface(const std::string & formulation)
  {
    const std::string error =
        "Mortar contact always geometrically and algebraically ghosts the interface.";
    EXPECT_THROW_MSG_CONTAINS(
        buildContactAction(GhostWholeInterfaceInput::True, formulation), std::runtime_error, error);
    EXPECT_THROW_MSG_CONTAINS(buildContactAction(GhostWholeInterfaceInput::False, formulation),
                              std::runtime_error,
                              error);
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

TEST_F(ContactRelationshipManagerTest, mortarContactActionRejectsGhostWholeInterface)
{
  testMortarContactActionRejectsGhostWholeInterface("mortar");
}

TEST_F(ContactRelationshipManagerTest, mortarPenaltyContactActionRejectsGhostWholeInterface)
{
  testMortarContactActionRejectsGhostWholeInterface("mortar_penalty");
}
