//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_KOKKOS_ENABLED

#include "gtest_include.h"

#include "MooseMain.h"
#include "AppFactory.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "FEProblemBase.h"
#include "MooseLinearVariableFV.h"
#include "LinearSystem.h"
#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"

// Fixture that creates an FEProblem with one nonlinear system, one aux system,
// and one linear system ("lin_sys").
class KokkosAllocationTest : public ::testing::Test
{
public:
  KokkosAllocationTest()
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
    _mesh->prepare(nullptr);
    _mesh->update();

    InputParameters problem_params = _factory.getValidParams("FEProblem");
    problem_params.set<MooseMesh *>("mesh") = _mesh.get();
    problem_params.set<std::string>(MooseBase::name_param) = "problem";
    problem_params.set<std::vector<NonlinearSystemName>>("nl_sys_names") = {"nl0"};
    problem_params.set<std::vector<LinearSystemName>>("linear_sys_names") = {"lin_sys"};
    _fe_problem = _factory.create<FEProblem>("FEProblem", "problem", problem_params);
    _fe_problem->createQRules(libMesh::QGAUSS, libMesh::FIRST, libMesh::FIRST, libMesh::FIRST);
    _app->actionWarehouse().problemBase() = _fe_problem;
  }

  void addFEVar(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("MooseVariable");
    _fe_problem->addVariable("MooseVariable", name, params);
  }

  void addLinearFVVar(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("MooseLinearVariableFVReal");
    params.set<SolverSystemName>("solver_sys") = "lin_sys";
    _fe_problem->addVariable("MooseLinearVariableFVReal", name, params);
  }

  void addLinearFVVarWithQp(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("MooseLinearVariableFVReal");
    params.set<SolverSystemName>("solver_sys") = "lin_sys";
    _fe_problem->addVariable("MooseLinearVariableFVReal", name, params);
    dynamic_cast<MooseLinearVariableFV<Real> &>(_fe_problem->getVariable(0, name))
        .requireQpComputations();
  }

  void addScalarAuxVar(const std::string & name)
  {
    InputParameters params = _factory.getValidParams("MooseVariableScalar");
    params.set<MooseEnum>("order") = "FIRST";
    params.set<MooseEnum>("family") = "SCALAR";
    _fe_problem->addAuxVariable("MooseVariableScalar", name, params);
  }

  // Distribute DOFs on all libMesh systems so the Kokkos::System constructor
  // can query n_local_dofs().  FEProblemBase::init() is not usable here because
  // it requires a live executioner; calling system().init() directly is sufficient.
  void initSystems()
  {
    for (unsigned int s = 0; s < _fe_problem->numNonlinearSystems(); ++s)
      _fe_problem->getNonlinearSystemBase(s).system().init();
    for (unsigned int s = 0; s < _fe_problem->numLinearSystems(); ++s)
      _fe_problem->getLinearSystem(s).system().init();
    _fe_problem->getAuxiliarySystem().system().init();
  }

  std::unique_ptr<MooseMesh> _mesh;
  std::shared_ptr<MooseApp> _app;
  Factory & _factory;
  std::shared_ptr<FEProblem> _fe_problem;
};

// Pure linear FV system - Kokkos::System only, no FESystem
TEST_F(KokkosAllocationTest, PureLinearFVSystemAllocatesSystemOnly)
{
  if (!_fe_problem->mesh().getKokkosMesh())
    GTEST_SKIP() << "Kokkos mesh not available";

  addLinearFVVar("u");

  initSystems();
  _fe_problem->initKokkos();

  const auto lin_num = _fe_problem->getLinearSystem(0).number();
  EXPECT_TRUE(_fe_problem->getKokkosSystems().isSlotConstructed(lin_num));
  EXPECT_FALSE(_fe_problem->getKokkosFESystems().isSlotConstructed(lin_num));
}

// Pure FE system - standalone FESystem, no Kokkos::System slot
TEST_F(KokkosAllocationTest, PureFESystemAllocatesFESystemOnly)
{
  if (!_fe_problem->mesh().getKokkosMesh())
    GTEST_SKIP() << "Kokkos mesh not available";

  addFEVar("v");

  initSystems();
  _fe_problem->initKokkos();

  const auto nl_num = _fe_problem->getNonlinearSystemBase(0).number();
  EXPECT_FALSE(_fe_problem->getKokkosSystems().isSlotConstructed(nl_num));
  EXPECT_TRUE(_fe_problem->getKokkosFESystems().isSlotConstructed(nl_num));
}

// Scalar-only system - Kokkos::System only, no FESystem
TEST_F(KokkosAllocationTest, ScalarOnlySystemAllocatesSystemOnly)
{
  if (!_fe_problem->mesh().getKokkosMesh())
    GTEST_SKIP() << "Kokkos mesh not available";

  addScalarAuxVar("s");

  initSystems();
  _fe_problem->initKokkos();

  const auto aux_num = _fe_problem->getAuxiliarySystem().number();
  EXPECT_TRUE(_fe_problem->getKokkosSystems().isSlotConstructed(aux_num));
  EXPECT_FALSE(_fe_problem->getKokkosFESystems().isSlotConstructed(aux_num));
}

// FV system with QP coupling - dual allocation (System + FESystem)
TEST_F(KokkosAllocationTest, LinearFVWithQpCouplingAllocatesBoth)
{
  if (!_fe_problem->mesh().getKokkosMesh())
    GTEST_SKIP() << "Kokkos mesh not available";

  addLinearFVVarWithQp("u");

  initSystems();
  _fe_problem->initKokkos();

  const auto lin_num = _fe_problem->getLinearSystem(0).number();
  EXPECT_TRUE(_fe_problem->getKokkosSystems().isSlotConstructed(lin_num));
  EXPECT_TRUE(_fe_problem->getKokkosFESystems().isSlotConstructed(lin_num));
}

// Mixed problem - FE system standalone, FV system is System-only
TEST_F(KokkosAllocationTest, MixedProblemCorrectAllocation)
{
  if (!_fe_problem->mesh().getKokkosMesh())
    GTEST_SKIP() << "Kokkos mesh not available";

  addFEVar("v");
  addLinearFVVar("u");

  initSystems();
  _fe_problem->initKokkos();

  const auto nl_num = _fe_problem->getNonlinearSystemBase(0).number();
  const auto lin_num = _fe_problem->getLinearSystem(0).number();

  // FE nonlinear system: FESystem only
  EXPECT_FALSE(_fe_problem->getKokkosSystems().isSlotConstructed(nl_num));
  EXPECT_TRUE(_fe_problem->getKokkosFESystems().isSlotConstructed(nl_num));

  // Linear FV system: System only
  EXPECT_TRUE(_fe_problem->getKokkosSystems().isSlotConstructed(lin_num));
  EXPECT_FALSE(_fe_problem->getKokkosFESystems().isSlotConstructed(lin_num));
}

// Empty system - no allocation for either array
TEST_F(KokkosAllocationTest, EmptySystemNoAllocation)
{
  if (!_fe_problem->mesh().getKokkosMesh())
    GTEST_SKIP() << "Kokkos mesh not available";

  // Add no variables to either the nonlinear or auxiliary systems.
  initSystems();
  _fe_problem->initKokkos();

  const auto nl_num = _fe_problem->getNonlinearSystemBase(0).number();
  EXPECT_FALSE(_fe_problem->getKokkosSystems().isSlotConstructed(nl_num));
  EXPECT_FALSE(_fe_problem->getKokkosFESystems().isSlotConstructed(nl_num));
}

#endif // MOOSE_KOKKOS_ENABLED
