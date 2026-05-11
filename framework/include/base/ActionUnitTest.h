//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest/gtest.h"

#include "MooseMesh.h"
#include "FEProblem.h"
#include "AppFactory.h"
#include "MooseMain.h"

/**
 * Base class for building basic unit tests for MOOSE actions
 *
 * This class follows the classic actions that are needed in order to build a simulation.
 * To build a unit test, inherit from this class and build your test using
 * the following template:
 *
 * In your .h file:
 *
 * class MyUnitTest : public ActionUnitTest
 * {
 * public:
 *   MyUnitTest() : ActionUnitTest("MyAppUnitApp")
 *   {
 *     buildAction();
 *   }
 *
 * protected:
 *
 *   void buildActions()
 *   {
 *     // Start with the validParams
 *     InputParameters pars = _action_factory.getValidParams("MyActionThatIAmTesting");
 *     // Set the parameters you need
 *     pars.set<bool>("do this thing") = true;
 *     // Build the action into the action warehouse (just like a meta-action would do)
 *
 *   }
 *
 *   void runActions()
 *   {
 *     // NOTE: if multiple tasks are required, you have to code them here
 *
 *     const auto task_name = "name of the task we want to test the action on";
 *     // Add the task: only needed if the task does not auto-register
 *     // Check that the task is registered
 *     mooseAssert(_action_factory.isRegisteredTask(task_name), "Should have registered the task");
 *     // Run the task
 *     _app->actionWarehouse().executeActionsWithAction(task_name);
 *   }
 *
 *   // member variable used later in the actual tests
 *   const MyActionIAmTesting * _action;
 * };
 *
 * In your .C file
 *
 * TEST_F(MyActionThatIAmTesting, name of the test)
 * {
 *   // Testing individual methods
 *   EXPECT_EQ("testing", _action->method(par1, par2));
 *   // Testing if an object has been created by action (if stored in the problem)
 *   EXPECT_EQ(true, _fe_problem->hasObjectType(name, 0));
 * }
 *
 * NOTE: Testing complex actions that build on other actions may require a deep
 *       knowledge of the setup phase in MOOSE. Use Debug/show_actions on a regular simulation
 *       to get more information on the setup
 */
class ActionUnitTest : public ::testing::Test
{
public:
  /**
   * @param app_name The name of client's application
   */
  ActionUnitTest(const std::string & app_name)
    : _app(Moose::createMooseApp(app_name, 0, nullptr)),
      _factory(_app->getFactory()),
      _action_factory(_app->getActionFactory())
  {
    buildMinimalObjects();
  }

  void SetUp() override
  {
    buildActions();
    runActions();
  }

protected:
  /// Override this to create the action(s) you want to test
  virtual void buildActions() = 0;

  /// Some base objects we often need to have the action act on them
  void buildMinimalObjects()
  {
    InputParameters mesh_params = _factory.getValidParams("GeneratedMesh");
    mesh_params.set<MooseEnum>("dim") = "3";
    mesh_params.set<unsigned int>("nx") = 2;
    mesh_params.set<unsigned int>("ny") = 2;
    mesh_params.set<unsigned int>("nz") = 2;
    _mesh = _factory.createUnique<MooseMesh>("GeneratedMesh", "name1", mesh_params);
    _mesh->setMeshBase(_mesh->buildMeshBaseObject());
    _mesh->buildMesh();

    InputParameters problem_params = _factory.getValidParams("FEProblem");
    problem_params.set<MooseMesh *>("mesh") = _mesh.get();
    problem_params.set<std::string>(MooseBase::name_param) = "name2";
    _fe_problem = _factory.create<FEProblem>("FEProblem", "problem", problem_params);

    _fe_problem->createQRules(libMesh::QGAUSS, libMesh::FIRST, libMesh::FIRST, libMesh::FIRST);

    _app->actionWarehouse().problemBase() = _fe_problem;
  }

  /// Override this to make the actions execute
  /// NOTE: many tasks will auto-register their task at creation, but for the others,
  /// do not forget to register them.
  /// To check if a task is registered, use:
  /// _action_factory.isRegisteredTask("auto_checkpoint_action")
  virtual void runActions() = 0;

  /// Convenience routine for adding an object in an Actions test
  template <typename T>
  T & addObject(const std::string & type, const std::string & name, InputParameters & params);

  std::unique_ptr<MooseMesh> _mesh;
  std::shared_ptr<MooseApp> _app;
  Factory & _factory;
  ActionFactory & _action_factory;
  std::shared_ptr<FEProblem> _fe_problem;
};

template <typename T>
T &
ActionUnitTest::addObject(const std::string & type,
                          const std::string & name,
                          InputParameters & params)
{
  auto objects = _fe_problem->addObject<T>(type, name, params);
  mooseAssert(objects.size() == 1, "Doesn't work with threading");
  return *objects[0];
}
