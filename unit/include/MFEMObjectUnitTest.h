//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest/gtest.h"

#include "MFEMMesh.h"
#include "MFEMProblem.h"
#include "AppFactory.h"
#include "MooseMain.h"

/**
 * Base class for building basic unit tests for MOOSE objects that can live alone (like user
 * objects, etc.)
 *
 * This class builds the basic objects that are needed in order to test a MOOSE object. Those are a
 * mesh and an FEProblem.  To build a unit test, inherit from this class and build your test using
 * the following template:
 *
 * In your .h file:
 *
 * class MyUnitTest : public MooseObjectUnitTest
 * {
 * public:
 *   MyUnitTest() : MooseObjectUnitTest("MyAppUnitApp")
 *   {
 *     // if you are using the old registration system, you want to register your objects using this
 *     // call. Otherwise, you do not need it.
 *     registerObjects(_factory);
 *     buildObjects();
 *   }
 *
 * protected:
 *   void registerObjects(Factory & factory)
 *   {
 *     // register your objects as usual, we have to be in a method like this so that the register
 *     // macros work
 *     registerUserObject(MyObjectThatIAmTesting);
 *   }
 *
 *   void buildObjects()
 *   {
 *     // build your object like so
 *     InputParameters pars = _factory.getValidParams("MyObjectThatIAmTesting");
 *     _fe_problem->addUserObject("MyObjectThatIAmTesting", "fp", uo_pars);
 *     _obj = &_fe_problem->getUserObject<MyObjectThatIAmTesting>("fp");
 *   }
 *
 *   // member variable used later in the actual tests
 *   const MyObjectThatIAmTesting * _obj;
 * };
 *
 * In your .C file
 *
 * TEST_F(MyObjectThatIAmTesting, test)
 * {
 *   EXPECT_EQ("testing", _obj->method(par1, par2));
 * }
 *
 * NOTE: Testing mesh-bound objects like Kernels, BCs, etc. is not possible with this class.
 */
class MFEMObjectUnitTest : public ::testing::Test
{
public:
  /**
   * @param app_name The name of client's application
   */
  MFEMObjectUnitTest(const std::string & app_name)
    : _app(Moose::createMooseApp(app_name, 0, nullptr)), _factory(_app->getFactory())
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    InputParameters mesh_params = _factory.getValidParams("MFEMMesh");
    mesh_params.set<MeshFileName>("file") = "data/beam-tet.mesh";
    _mfem_mesh_ptr = _factory.createUnique<MFEMMesh>("MFEMMesh", "moose_mesh", mesh_params);
    _mfem_mesh_ptr->setMeshBase(_mfem_mesh_ptr->buildMeshBaseObject());
    _mfem_mesh_ptr->buildMesh();

    InputParameters problem_params = _factory.getValidParams("MFEMProblem");
    problem_params.set<MooseMesh *>("mesh") = _mfem_mesh_ptr.get();
    problem_params.set<std::string>("_object_name") = "name2";
    _mfem_problem = _factory.create<MFEMProblem>("MFEMProblem", "problem", problem_params);

    _app->actionWarehouse().problemBase() = _mfem_problem;
  }

  template <typename T>
  T & addObject(const std::string & type, const std::string & name, InputParameters & params);

  std::unique_ptr<MFEMMesh> _mfem_mesh_ptr;
  std::shared_ptr<MooseApp> _app;
  Factory & _factory;
  std::shared_ptr<MFEMProblem> _mfem_problem;
};

template <typename T>
T &
MFEMObjectUnitTest::addObject(const std::string & type,
                              const std::string & name,
                              InputParameters & params)
{
  auto objects = _mfem_problem->addObject<T>(type, name, params);
  mooseAssert(objects.size() == 1, "Doesn't work with threading");
  return *objects[0];
}
