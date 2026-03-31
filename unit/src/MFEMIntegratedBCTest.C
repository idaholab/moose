//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mfem-common.hpp"
#include "libmesh/restore_warnings.h"
#include "EquationSystem.h"
#include "MFEMObjectUnitTest.h"
#include "MFEMBoundaryIntegratedBC.h"
#include "MFEMVectorBoundaryIntegratedBC.h"
#include "MFEMBoundaryNormalIntegratedBC.h"
#include "MFEMConvectiveHeatFluxBC.h"
#include "MFEMDiffusionKernel.h"
#include "MFEMIntegratedBC.h"

namespace
{
class ZeroBoundaryNonlinearIntegrator : public mfem::NonlinearFormIntegrator
{
public:
  void AssembleFaceVector(const mfem::FiniteElement & el1,
                          const mfem::FiniteElement &,
                          mfem::FaceElementTransformations &,
                          const mfem::Vector &,
                          mfem::Vector & elvect) override
  {
    elvect.SetSize(el1.GetDof());
    elvect = 0.0;
  }

  void AssembleFaceGrad(const mfem::FiniteElement & el1,
                        const mfem::FiniteElement &,
                        mfem::FaceElementTransformations &,
                        const mfem::Vector &,
                        mfem::DenseMatrix & elmat) override
  {
    elmat.SetSize(el1.GetDof());
    elmat = 0.0;
  }
};

class TestOffDiagonalLinearIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams()
  {
    auto params = MFEMIntegratedBC::validParams();
    params.addParam<VariableName>("trial_variable",
                                  "Trial variable this boundary condition acts on.");
    params.addClassDescription(
        "Test-only MFEM integrated boundary condition with a linear off-diagonal mass term.");
    return params;
  }

  TestOffDiagonalLinearIntegratedBC(const InputParameters & parameters)
    : MFEMIntegratedBC(parameters),
      _trial_var_name(getParam<VariableName>("trial_variable")),
      _coef(1.0)
  {
  }

  const std::string & getTrialVariableName() const override { return _trial_var_name; }

  mfem::BilinearFormIntegrator * createBFIntegrator() override
  {
    return new mfem::BoundaryMassIntegrator(_coef);
  }

private:
  const VariableName _trial_var_name;
  mfem::ConstantCoefficient _coef;
};

class TestOffDiagonalNonlinearIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams()
  {
    auto params = MFEMIntegratedBC::validParams();
    params.addParam<VariableName>("trial_variable",
                                  "Trial variable this boundary condition acts on.");
    params.addClassDescription(
        "Test-only MFEM integrated boundary condition with an off-diagonal nonlinear term.");
    return params;
  }

  TestOffDiagonalNonlinearIntegratedBC(const InputParameters & parameters)
    : MFEMIntegratedBC(parameters), _trial_var_name(getParam<VariableName>("trial_variable"))
  {
  }

  const std::string & getTrialVariableName() const override { return _trial_var_name; }

  mfem::NonlinearFormIntegrator * createNLIntegrator() override
  {
    return new ZeroBoundaryNonlinearIntegrator();
  }

private:
  const VariableName _trial_var_name;
};

class TestEquationSystem : public Moose::MFEM::EquationSystem
{
public:
  void initAndBuild(Moose::MFEM::GridFunctions & gridfunctions,
                    Moose::MFEM::ComplexGridFunctions & cmplx_gridfunctions,
                    mfem::AssemblyLevel assembly_level)
  {
    Init(gridfunctions, cmplx_gridfunctions, assembly_level);
    BuildEquationSystem();
  }
};
}

registerMooseObject("MooseUnitApp", TestOffDiagonalLinearIntegratedBC);
registerMooseObject("MooseUnitApp", TestOffDiagonalNonlinearIntegratedBC);

class MFEMIntegratedBCTest : public MFEMObjectUnitTest
{
public:
  MFEMIntegratedBCTest() : MFEMObjectUnitTest("MooseUnitApp")
  {
    // Register dummy (Par)GridFunctions for the variables the BCs apply to
    auto pm = _mfem_mesh_ptr->getMFEMParMeshPtr().get();
    mfem::common::H1_FESpace fe(pm, 1);
    mfem::GridFunction gf(&fe);
    _mfem_problem->getProblemData().gridfunctions.Register(
        "test_variable_name", std::make_shared<mfem::ParGridFunction>(pm, &gf));
    _mfem_problem->getProblemData().gridfunctions.Register(
        "trial_variable_name", std::make_shared<mfem::ParGridFunction>(pm, &gf));
  }

protected:
  template <typename T>
  std::shared_ptr<T> addSharedObject(const std::string & type,
                                     const std::string & name,
                                     InputParameters & params)
  {
    auto objects = _mfem_problem->addObject<T>(type, name, params);
    mooseAssert(objects.size() == 1, "Doesn't work with threading");
    return objects[0];
  }
};

/**
 * Test MFEMBoundaryNormalIntegratedBC creates an mfem::BoundaryNormalLFIntegrator
 * successfully.
 */
TEST_F(MFEMIntegratedBCTest, MFEMVectorNormalIntegratedConstantBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMBoundaryNormalIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  MFEMBoundaryNormalIntegratedBC & integrated_bc =
      addObject<MFEMBoundaryNormalIntegratedBC>("MFEMBoundaryNormalIntegratedBC", "bc1", bc_params);

  // Test MFEMBoundaryNormalIntegratedBC returns an integrator of the expected type
  auto lf_integrator =
      dynamic_cast<mfem::BoundaryNormalLFIntegrator *>(integrated_bc.createLFIntegrator());
  ASSERT_TRUE(lf_integrator != nullptr);
  delete lf_integrator;

  auto blf_integrator = integrated_bc.createBFIntegrator();
  ASSERT_TRUE(blf_integrator == nullptr);
  delete blf_integrator;
}

/**
 * Test MFEMBoundaryNormalIntegratedBC creates an mfem::BoundaryNormalLFIntegrator
 * successfully.
 */
TEST_F(MFEMIntegratedBCTest, MFEMBoundaryNormalIntegratedBC)
{
  // Construct boundary condition
  InputParameters func_params = _factory.getValidParams("ParsedVectorFunction");
  func_params.set<std::string>("expression_x") = "x + y";
  func_params.set<std::string>("expression_y") = "x + y + 1";
  func_params.set<std::string>("expression_z") = "x + y + 2";
  _mfem_problem->addFunction("ParsedVectorFunction", "func1", func_params);
  InputParameters bc_params = _factory.getValidParams("MFEMBoundaryNormalIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "func1";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  MFEMBoundaryNormalIntegratedBC & integrated_bc =
      addObject<MFEMBoundaryNormalIntegratedBC>("MFEMBoundaryNormalIntegratedBC", "bc1", bc_params);

  // Test MFEMBoundaryNormalIntegratedBC returns an integrator of the expected type
  auto lf_integrator =
      dynamic_cast<mfem::BoundaryNormalLFIntegrator *>(integrated_bc.createLFIntegrator());
  ASSERT_TRUE(lf_integrator != nullptr);
  delete lf_integrator;

  auto blf_integrator = integrated_bc.createBFIntegrator();
  ASSERT_TRUE(blf_integrator == nullptr);
  delete blf_integrator;
}

/**
 * Test MFEMBoundaryIntegratedBC creates the expected mfem::BoundaryIntegrator
 * successfully.
 */
TEST_F(MFEMIntegratedBCTest, MFEMBoundaryIntegratedBC)
{
  // Build required BC inputs
  InputParameters coef_params = _factory.getValidParams("MFEMGenericFunctorMaterial");
  coef_params.set<std::vector<std::string>>("prop_names") = {"coef1"};
  coef_params.set<std::vector<MFEMScalarCoefficientName>>("prop_values") = {"3.0"};
  _mfem_problem->addFunctorMaterial("MFEMGenericFunctorMaterial", "material1", coef_params);

  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMBoundaryIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMScalarCoefficientName>("coefficient") = "coef1";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  MFEMBoundaryIntegratedBC & integrated_bc =
      addObject<MFEMBoundaryIntegratedBC>("MFEMBoundaryIntegratedBC", "bc1", bc_params);

  // Test MFEMBoundaryIntegratedBC returns an integrator of the expected type
  auto lf_integrator =
      dynamic_cast<mfem::BoundaryLFIntegrator *>(integrated_bc.createLFIntegrator());
  ASSERT_NE(lf_integrator, nullptr);
  delete lf_integrator;

  auto blf_integrator = integrated_bc.createBFIntegrator();
  ASSERT_EQ(blf_integrator, nullptr);
  delete blf_integrator;
}

/**
 * Test MFEMConvectiveHeatFluxBC creates the expected mfem::BoundaryIntegrators successfully.
 */
TEST_F(MFEMIntegratedBCTest, MFEMConvectiveHeatFluxBC)
{
  // Build required BC inputs
  InputParameters htc_params = _factory.getValidParams("ParsedFunction");
  htc_params.set<std::string>("expression") = "1.0";
  _mfem_problem->addFunction("ParsedFunction", "htc", htc_params);
  _mfem_problem->getFunction("htc").initialSetup();
  InputParameters Tinf_params = _factory.getValidParams("ParsedFunction");
  Tinf_params.set<std::string>("expression") = "3.0";
  _mfem_problem->addFunction("ParsedFunction", "Tinf", Tinf_params);

  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMConvectiveHeatFluxBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<MFEMScalarCoefficientName>("heat_transfer_coefficient") = "htc";
  bc_params.set<MFEMScalarCoefficientName>("T_infinity") = "Tinf";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  MFEMConvectiveHeatFluxBC & integrated_bc =
      addObject<MFEMConvectiveHeatFluxBC>("MFEMConvectiveHeatFluxBC", "bc1", bc_params);

  // Test MFEMConvectiveHeatFluxBC returns an integrator of the expected type
  auto lf_integrator =
      dynamic_cast<mfem::BoundaryLFIntegrator *>(integrated_bc.createLFIntegrator());
  ASSERT_NE(lf_integrator, nullptr);
  delete lf_integrator;

  auto blf_integrator =
      dynamic_cast<mfem::BoundaryMassIntegrator *>(integrated_bc.createBFIntegrator());
  ASSERT_NE(blf_integrator, nullptr);
  delete blf_integrator;
}

TEST_F(MFEMIntegratedBCTest, MFEMVectorBoundaryIntegratedConstantBC)
{
  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorBoundaryIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & bc =
      addObject<MFEMVectorBoundaryIntegratedBC>("MFEMVectorBoundaryIntegratedBC", "bc1", bc_params);

  // Test MFEMVectorBoundaryIntegratedBC returns an integrator of the expected type
  auto lf_integrator = dynamic_cast<mfem::VectorBoundaryLFIntegrator *>(bc.createLFIntegrator());
  ASSERT_NE(lf_integrator, nullptr);
  delete lf_integrator;

  auto blf_integrator = bc.createBFIntegrator();
  ASSERT_EQ(blf_integrator, nullptr);
  delete blf_integrator;
}

TEST_F(MFEMIntegratedBCTest, MFEMVectorBoundaryIntegratedBC)
{
  // Build required BC inputs
  InputParameters func_params = _factory.getValidParams("ParsedVectorFunction");
  func_params.set<std::string>("expression_x") = "x + y";
  func_params.set<std::string>("expression_y") = "x + y + 1";
  func_params.set<std::string>("expression_z") = "x + y + 2";
  _mfem_problem->addFunction("ParsedVectorFunction", "func1", func_params);

  // Construct boundary condition
  InputParameters bc_params = _factory.getValidParams("MFEMVectorBoundaryIntegratedBC");
  bc_params.set<VariableName>("variable") = "test_variable_name";
  bc_params.set<std::vector<BoundaryName>>("boundary") = {"1"};
  bc_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "func1";
  auto & bc =
      addObject<MFEMVectorBoundaryIntegratedBC>("MFEMVectorBoundaryIntegratedBC", "bc1", bc_params);

  // Test MFEMVectorBoundaryIntegratedBC returns an integrator of the expected type
  auto lf_integrator = dynamic_cast<mfem::VectorBoundaryLFIntegrator *>(bc.createLFIntegrator());
  ASSERT_NE(lf_integrator, nullptr);
  delete lf_integrator;

  auto blf_integrator = bc.createBFIntegrator();
  ASSERT_EQ(blf_integrator, nullptr);
  delete blf_integrator;
}

TEST_F(MFEMIntegratedBCTest, RejectsOffDiagonalNonlinearIntegratedBCWhenBuildingEquationSystem)
{
  InputParameters diag_test_params = _factory.getValidParams("MFEMDiffusionKernel");
  diag_test_params.set<VariableName>("variable") = "test_variable_name";
  diag_test_params.set<MFEMScalarCoefficientName>("coefficient") = "1.0";

  InputParameters nonlinear_params =
      _factory.getValidParams("TestOffDiagonalNonlinearIntegratedBC");
  nonlinear_params.set<VariableName>("variable") = "test_variable_name";
  nonlinear_params.set<VariableName>("trial_variable") = "trial_variable_name";
  nonlinear_params.set<std::vector<BoundaryName>>("boundary") = {"1"};

  auto diag_test =
      addSharedObject<MFEMDiffusionKernel>("MFEMDiffusionKernel", "diag_test_bc", diag_test_params);
  auto nonlinear = addSharedObject<TestOffDiagonalNonlinearIntegratedBC>(
      "TestOffDiagonalNonlinearIntegratedBC", "nonlinear_offdiag_bc", nonlinear_params);

  TestEquationSystem eqn_system;
  eqn_system.AddKernel(diag_test);
  eqn_system.AddIntegratedBC(nonlinear);

  try
  {
    eqn_system.initAndBuild(_mfem_problem->getProblemData().gridfunctions,
                            _mfem_problem->getProblemData().cmplx_gridfunctions,
                            mfem::AssemblyLevel::LEGACY);
    FAIL() << "Expected off-diagonal nonlinear MFEM integrated boundary condition to be rejected";
  }
  catch (const std::runtime_error & error)
  {
    EXPECT_TRUE(std::string(error.what()).find("not currently implemented") != std::string::npos);
  }
}

TEST_F(MFEMIntegratedBCTest, AcceptsLinearOffDiagonalIntegratedBCWhenBuildingEquationSystem)
{
  InputParameters diag_test_params = _factory.getValidParams("MFEMDiffusionKernel");
  diag_test_params.set<VariableName>("variable") = "test_variable_name";
  diag_test_params.set<MFEMScalarCoefficientName>("coefficient") = "1.0";

  InputParameters diag_trial_params = _factory.getValidParams("MFEMDiffusionKernel");
  diag_trial_params.set<VariableName>("variable") = "trial_variable_name";
  diag_trial_params.set<MFEMScalarCoefficientName>("coefficient") = "1.0";

  InputParameters linear_params = _factory.getValidParams("TestOffDiagonalLinearIntegratedBC");
  linear_params.set<VariableName>("variable") = "test_variable_name";
  linear_params.set<VariableName>("trial_variable") = "trial_variable_name";
  linear_params.set<std::vector<BoundaryName>>("boundary") = {"1"};

  auto diag_test = addSharedObject<MFEMDiffusionKernel>(
      "MFEMDiffusionKernel", "diag_test_bc_2", diag_test_params);
  auto diag_trial = addSharedObject<MFEMDiffusionKernel>(
      "MFEMDiffusionKernel", "diag_trial_bc_2", diag_trial_params);
  auto linear = addSharedObject<TestOffDiagonalLinearIntegratedBC>(
      "TestOffDiagonalLinearIntegratedBC", "linear_offdiag_bc", linear_params);

  TestEquationSystem eqn_system;
  eqn_system.AddKernel(diag_test);
  // Keep a diagonal contribution on the trial variable so this exercises the supported mixed
  // 2x2 system path rather than a case where trial_variable_name is only an eliminated coupling.
  eqn_system.AddKernel(diag_trial);
  eqn_system.AddIntegratedBC(linear);

  EXPECT_NO_THROW(eqn_system.initAndBuild(_mfem_problem->getProblemData().gridfunctions,
                                          _mfem_problem->getProblemData().cmplx_gridfunctions,
                                          mfem::AssemblyLevel::LEGACY));
}

#endif
