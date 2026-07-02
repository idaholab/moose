//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseMain.h"
#include "AppFactory.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseVariableFieldBase.h"
#include "INSFVPressureVariable.h"
#include "INSFVVariable.h"

class INSFVVariableTest : public ::testing::Test
{
public:
  INSFVVariableTest()
    : _app(Moose::createMooseApp("NavierStokesUnitApp", 0, nullptr)), _factory(_app->getFactory())
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    InputParameters mesh_params = _factory.getValidParams("GeneratedMesh");
    mesh_params.set<MooseEnum>("dim") = "2";
    mesh_params.set<unsigned int>("nx") = 2;
    mesh_params.set<unsigned int>("ny") = 2;
    _mesh = _factory.createUnique<MooseMesh>("GeneratedMesh", "moose_mesh", mesh_params);
    _mesh->setMeshBase(_mesh->buildMeshBaseObject());
    _mesh->buildMesh();

    InputParameters problem_params = _factory.getValidParams("FEProblem");
    problem_params.set<MooseMesh *>("mesh") = _mesh.get();
    problem_params.set<std::string>(MooseBase::name_param) = "problem";
    _fe_problem = _factory.create<FEProblem>("FEProblem", "problem", problem_params);
    _fe_problem->createQRules(libMesh::QGAUSS, libMesh::FIRST, libMesh::FIRST, libMesh::FIRST);
    _app->actionWarehouse().problemBase() = _fe_problem;
  }

  // Add an INSFVPressureVariable to the default nonlinear system
  MooseVariableFieldBase & addINSFVVar(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("INSFVPressureVariable");
    _fe_problem->addVariable("INSFVPressureVariable", name, params);
    return _fe_problem->getVariable(0, name);
  }

  std::unique_ptr<MooseMesh> _mesh;
  std::shared_ptr<MooseApp> _app;
  Factory & _factory;
  std::shared_ptr<FEProblem> _fe_problem;
};

// INSFVVariable default does not compute QP data
TEST_F(INSFVVariableTest, INSFVVariableDefaultNoQpData)
{
  auto & var = addINSFVVar("p");
  EXPECT_FALSE(var.usesQpBasedLoops());
}

// INSFVVariable opts in via requireQpComputations()
TEST_F(INSFVVariableTest, INSFVVariableOptsInViaRequireQpComputations)
{
  auto & var = addINSFVVar("p");
  dynamic_cast<INSFVVariable &>(var).requireQpComputations();
  EXPECT_TRUE(var.usesQpBasedLoops());
}

// INSFVVariable always uses FV assembly
TEST_F(INSFVVariableTest, INSFVVariableAlwaysTrueFVAssembly)
{
  auto & var = addINSFVVar("p");
  EXPECT_TRUE(var.usesGeometricInfoBasedLoops());
}

// usesGeometricInfoBasedLoops() on INSFVVariable is unaffected by requireQpComputations()
TEST_F(INSFVVariableTest, INSFVVariableTrueFVAssemblyUnaffectedByRequireQp)
{
  auto & var = addINSFVVar("p");
  dynamic_cast<INSFVVariable &>(var).requireQpComputations();
  EXPECT_TRUE(var.usesGeometricInfoBasedLoops());
}
