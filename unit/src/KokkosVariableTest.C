//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "MooseMain.h"
#include "AppFactory.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseVariableBase.h"
#include "MooseVariableFieldBase.h"
#include "MooseLinearVariableFV.h"
#include "MooseVariableFV.h"
#include "MooseVariableScalar.h"
#include "AuxKernel.h"

// Minimal AuxKernel that couples to a field variable.
// Verifies that Coupleable::Coupleable() calls requireQpComputations()
// on MooseLinearVariableFV<Real>.
class TestLinearFVCoupledAux : public AuxKernel
{
public:
  static InputParameters validParams()
  {
    InputParameters params = AuxKernel::validParams();
    params.addRequiredCoupledVar("coupled", "The coupled variable");
    return params;
  }

  TestLinearFVCoupledAux(const InputParameters & params)
    : AuxKernel(params), _coupled_value(coupledValue("coupled"))
  {
  }

protected:
  Real computeValue() override { return _coupled_value[_qp]; }

private:
  const VariableValue & _coupled_value;
};

registerMooseObject("MooseUnitApp", TestLinearFVCoupledAux);

class KokkosVariableTest : public ::testing::Test
{
public:
  KokkosVariableTest()
    : _app(Moose::createMooseApp("MooseUnitApp", 0, nullptr)), _factory(_app->getFactory())
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
    problem_params.set<std::vector<NonlinearSystemName>>("nl_sys_names") = {"nl0"};
    problem_params.set<std::vector<LinearSystemName>>("linear_sys_names") = {"lin_sys"};
    _fe_problem = _factory.create<FEProblem>("FEProblem", "problem", problem_params);
    _fe_problem->createQRules(libMesh::QGAUSS, libMesh::FIRST, libMesh::FIRST, libMesh::FIRST);
    _app->actionWarehouse().problemBase() = _fe_problem;
  }

  // Add a standard FE variable to the default nonlinear system
  MooseVariableFieldBase & addFEVar(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("MooseVariable");
    _fe_problem->addVariable("MooseVariable", name, params);
    return _fe_problem->getVariable(0, name);
  }

  // Add an FV variable to the default nonlinear system
  MooseVariableFieldBase & addFVVar(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("MooseVariableFVReal");
    _fe_problem->addVariable("MooseVariableFVReal", name, params);
    return _fe_problem->getVariable(0, name);
  }

  // Add a linear FV variable to the linear system
  MooseVariableFieldBase & addLinearFVVar(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("MooseLinearVariableFVReal");
    params.set<SolverSystemName>("solver_sys") = "lin_sys";
    _fe_problem->addVariable("MooseLinearVariableFVReal", name, params);
    return _fe_problem->getVariable(0, name);
  }

  // MONOMIAL CONSTANT so the kernel is elemental — required when coupling to a cell-centred FV var.
  MooseVariableFieldBase & addAuxFEVar(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("MooseVariable");
    params.set<MooseEnum>("family") = "MONOMIAL";
    params.set<MooseEnum>("order") = "CONSTANT";
    _fe_problem->addAuxVariable("MooseVariable", name, params);
    return _fe_problem->getVariable(0, name, Moose::VarKindType::VAR_AUXILIARY);
  }

  // Add a scalar variable to the auxiliary system
  MooseVariableScalar & addScalarVar(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("MooseVariableScalar");
    params.set<MooseEnum>("order") = "FIRST";
    params.set<MooseEnum>("family") = "SCALAR";
    _fe_problem->addAuxVariable("MooseVariableScalar", name, params);
    return _fe_problem->getScalarVariable(0, name);
  }

  std::unique_ptr<MooseMesh> _mesh;
  std::shared_ptr<MooseApp> _app;
  Factory & _factory;
  std::shared_ptr<FEProblem> _fe_problem;
};

// --- computesQpData() tests ---

TEST_F(KokkosVariableTest, FEVariableComputesQpData)
{
  auto & var = addFEVar("v");
  EXPECT_TRUE(var.computesQpData());
}

TEST_F(KokkosVariableTest, FVVariableBaseComputesQpData)
{
  auto & var = addFVVar("v");
  EXPECT_TRUE(var.computesQpData());
}

TEST_F(KokkosVariableTest, LinearFVDefaultNoQpData)
{
  auto & var = addLinearFVVar("u");
  EXPECT_FALSE(var.computesQpData());
}

TEST_F(KokkosVariableTest, LinearFVOptsInViaRequireQpComputations)
{
  auto & var = addLinearFVVar("u");
  dynamic_cast<MooseLinearVariableFV<Real> &>(var).requireQpComputations();
  EXPECT_TRUE(var.computesQpData());
}

TEST_F(KokkosVariableTest, ScalarVariableNoQpData)
{
  auto & var = addScalarVar("s");
  EXPECT_FALSE(var.computesQpData());
}

// Coupleable construction with a MooseLinearVariableFV<Real> coupled variable
// triggers requireQpComputations() on that variable.
TEST_F(KokkosVariableTest, CoupleableTriggerRequireQpComputations)
{
  auto & lin_var = addLinearFVVar("u_lin");
  EXPECT_FALSE(lin_var.computesQpData());

  // Add an aux variable as the kernel write target
  addAuxFEVar("dummy_aux");

  // Construct a Coupleable (AuxKernel) that couples to the linear FV variable.
  // The Coupleable constructor processes coupled variables and calls
  // requireQpComputations() on any MooseLinearVariableFV<Real> it finds.
  InputParameters params = _factory.getValidParams("TestLinearFVCoupledAux");
  params.set<AuxVariableName>("variable") = "dummy_aux";
  params.set<std::vector<VariableName>>("coupled") = {"u_lin"};
  _fe_problem->addAuxKernel("TestLinearFVCoupledAux", "kern", params);

  EXPECT_TRUE(lin_var.computesQpData());
}

// --- doesTrueFVAssembly() tests ---

TEST_F(KokkosVariableTest, FEVariableNoTrueFVAssembly)
{
  auto & var = addFEVar("v");
  EXPECT_FALSE(var.doesTrueFVAssembly());
}

TEST_F(KokkosVariableTest, FVVariableBaseNoTrueFVAssembly)
{
  auto & var = addFVVar("v");
  EXPECT_FALSE(var.doesTrueFVAssembly());
}

TEST_F(KokkosVariableTest, LinearFVAlwaysTrueFVAssembly)
{
  auto & var = addLinearFVVar("u");
  EXPECT_TRUE(var.doesTrueFVAssembly());
}

TEST_F(KokkosVariableTest, LinearFVTrueFVAssemblyUnaffectedByRequireQp)
{
  auto & var = addLinearFVVar("u");
  dynamic_cast<MooseLinearVariableFV<Real> &>(var).requireQpComputations();
  EXPECT_TRUE(var.doesTrueFVAssembly());
}

TEST_F(KokkosVariableTest, ScalarVariableNoTrueFVAssembly)
{
  auto & var = addScalarVar("s");
  EXPECT_FALSE(var.doesTrueFVAssembly());
}
