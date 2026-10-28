//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "EquationSystem.h"
#include "MFEMConservativeConvectionKernel.h"
#include "MFEMConvectionKernel.h"
#include "MFEMCurlCurlKernel.h"
#include "MFEMDerivativeKernel.h"
#include "MFEMDGDiffusionBR2Kernel.h"
#include "MFEMDGElasticityKernel.h"
#include "MFEMDGTraceKernel.h"
#include "MFEMDiffusionKernel.h"
#include "MFEMDivDivKernel.h"
#include "MFEMDomainLFGradKernel.h"
#include "MFEMGradientKernel.h"
#include "MFEMGroupConvectionKernel.h"
#include "MFEMLinearElasticityKernel.h"
#include "MFEMMixedBilinearFormKernel.h"
#include "MFEMMixedCrossCurlCurlKernel.h"
#include "MFEMMixedCrossCurlGradKernel.h"
#include "MFEMMixedCrossCurlKernel.h"
#include "MFEMMixedCrossGradCurlKernel.h"
#include "MFEMMixedCrossGradGradKernel.h"
#include "MFEMMixedCrossGradKernel.h"
#include "MFEMMixedCrossProductKernel.h"
#include "MFEMMixedCurlCurlKernel.h"
#include "MFEMMixedCurlKernel.h"
#include "MFEMMixedDirectionalDerivativeKernel.h"
#include "MFEMMixedDivGradKernel.h"
#include "MFEMMixedDotProductKernel.h"
#include "MFEMMixedGradDivKernel.h"
#include "MFEMMixedScalarCrossCurlKernel.h"
#include "MFEMMixedScalarCrossGradKernel.h"
#include "MFEMMixedScalarCrossProductKernel.h"
#include "MFEMMixedScalarCurlKernel.h"
#include "MFEMMixedScalarDerivativeKernel.h"
#include "MFEMMixedScalarDivergenceKernel.h"
#include "MFEMMixedScalarMassKernel.h"
#include "MFEMMixedScalarWeakCrossProductKernel.h"
#include "MFEMMixedScalarWeakCurlCrossKernel.h"
#include "MFEMMixedScalarWeakDerivativeKernel.h"
#include "MFEMMixedScalarWeakDivergenceKernel.h"
#include "MFEMMixedScalarWeakGradientKernel.h"
#include "MFEMMixedVectorCurlKernel.h"
#include "MFEMMixedVectorDivergenceKernel.h"
#include "MFEMMixedVectorGradientKernel.h"
#include "MFEMMixedGradGradKernel.h"
#include "MFEMMixedScalarWeakCurlKernel.h"
#include "MFEMMixedVectorMassKernel.h"
#include "MFEMMixedVectorProductKernel.h"
#include "MFEMMixedVectorWeakCurlKernel.h"
#include "MFEMMixedVectorWeakDivergenceKernel.h"
#include "MFEMMixedWeakCurlCrossKernel.h"
#include "MFEMMixedWeakDivCrossKernel.h"
#include "MFEMMixedWeakGradDotKernel.h"
#include "MFEMNonconservativeDGTraceKernel.h"
#include "MFEMVectorCurlCurlKernel.h"
#include "MFEMVectorDiffusionKernel.h"
#include "MFEMVectorDivergenceKernel.h"
#include "MFEMVectorDomainLFGradKernel.h"
#include "MFEMVectorDomainLFKernel.h"
#include "MFEMVectorFECurlKernel.h"
#include "MFEMVectorFEDomainLFCurlKernel.h"
#include "MFEMVectorFEDomainLFDivKernel.h"
#include "MFEMVectorFEDomainLFKernel.h"
#include "MFEMVectorFEMassKernel.h"
#include "MFEMVectorFEWeakDivergenceKernel.h"
#include "MFEMVectorMassKernel.h"
#include "MFEMWhiteGaussianNoiseDomainLFKernel.h"

namespace
{
class ZeroNonlinearIntegrator : public mfem::NonlinearFormIntegrator
{
public:
  void AssembleElementVector(const mfem::FiniteElement & el,
                             mfem::ElementTransformation &,
                             const mfem::Vector &,
                             mfem::Vector & elvect) override
  {
    elvect.SetSize(el.GetDof());
    elvect = 0.0;
  }

  void AssembleElementGrad(const mfem::FiniteElement & el,
                           mfem::ElementTransformation &,
                           const mfem::Vector &,
                           mfem::DenseMatrix & elmat) override
  {
    elmat.SetSize(el.GetDof());
    elmat = 0.0;
  }
};

class TestOffDiagonalLinearKernel : public MFEMMixedBilinearFormKernel
{
public:
  static InputParameters validParams()
  {
    auto params = MFEMMixedBilinearFormKernel::validParams();
    params.addClassDescription("Test-only MFEM mixed kernel with a linear off-diagonal mass term.");
    return params;
  }

  TestOffDiagonalLinearKernel(const InputParameters & parameters)
    : MFEMMixedBilinearFormKernel(parameters)
  {
  }

  mfem::BilinearFormIntegrator * createMBFIntegrator() override
  {
    return new mfem::MixedScalarMassIntegrator;
  }
};

class TestOffDiagonalNonlinearKernel : public MFEMMixedBilinearFormKernel
{
public:
  static InputParameters validParams()
  {
    auto params = MFEMMixedBilinearFormKernel::validParams();
    params.addClassDescription(
        "Test-only MFEM mixed kernel with an off-diagonal nonlinear contribution.");
    return params;
  }

  TestOffDiagonalNonlinearKernel(const InputParameters & parameters)
    : MFEMMixedBilinearFormKernel(parameters)
  {
  }

  mfem::NonlinearFormIntegrator * createNLIntegrator() override
  {
    return new ZeroNonlinearIntegrator();
  }
};

class TestDiagonalNonlinearKernel : public MFEMKernel
{
public:
  static InputParameters validParams()
  {
    auto params = MFEMKernel::validParams();
    params.addClassDescription("Test-only MFEM kernel with a diagonal nonlinear contribution.");
    return params;
  }

  TestDiagonalNonlinearKernel(const InputParameters & parameters) : MFEMKernel(parameters) {}

  mfem::NonlinearFormIntegrator * createNLIntegrator() override
  {
    return new ZeroNonlinearIntegrator();
  }
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

  void setAssemblyLevel(mfem::AssemblyLevel assembly_level) { _assembly_level = assembly_level; }
};
}

registerMooseObject("MooseUnitApp", TestOffDiagonalLinearKernel);
registerMooseObject("MooseUnitApp", TestOffDiagonalNonlinearKernel);
registerMooseObject("MooseUnitApp", TestDiagonalNonlinearKernel);

class MFEMKernelTest : public MFEMObjectUnitTest
{
public:
  MFEMKernelTest() : MFEMObjectUnitTest("MooseUnitApp")
  {
    // Register dummy (Par)GridFunctions for kernels to apply to
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
  std::shared_ptr<T>
  addSharedObject(const std::string & type, const std::string & name, InputParameters & params)
  {
    auto objects = _mfem_problem->addObject<T>(type, name, params);
    mooseAssert(objects.size() == 1, "Doesn't work with threading");
    return objects[0];
  }
};

/**
 * Test MFEMCurlCurlKernel creates an mfem::CurlCurlIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMCurlCurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMCurlCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  MFEMCurlCurlKernel & kernel =
      addObject<MFEMCurlCurlKernel>("MFEMCurlCurlKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::CurlCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_TRUE(integrator != nullptr);
  delete integrator;
}

/**
 * Test MFEMDiffusionKernel creates an mfem::DiffusionIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMDiffusionKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMDiffusionKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  kernel_params.set<std::vector<SubdomainName>>("block") = {"2"};
  MFEMDiffusionKernel & kernel =
      addObject<MFEMDiffusionKernel>("MFEMDiffusionKernel", "kernel1", kernel_params);
  // Test MFEMKernel marker array has been constructed as expected
  ASSERT_EQ(kernel.getSubdomainMarkers(), mfem::Array<int>({0, 1}));
  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::DiffusionIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMDiffusionKernel creates an mfem::DiffusionIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMDivDivKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMDivDivKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel = addObject<MFEMDivDivKernel>("MFEMDivDivKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::DivDivIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMLinearElasticityKernel creates an mfem::ElasticityIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMLinearElasticityKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMLinearElasticityKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("lambda") = "2.0";
  kernel_params.set<MFEMScalarCoefficientName>("mu") = "3.0";
  MFEMLinearElasticityKernel & kernel =
      addObject<MFEMLinearElasticityKernel>("MFEMLinearElasticityKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::ElasticityIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedVectorGradientKernel creates an mfem::MixedVectorGradientIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedVectorGradientKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedVectorGradientKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  MFEMMixedVectorGradientKernel & kernel = addObject<MFEMMixedVectorGradientKernel>(
      "MFEMMixedVectorGradientKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedVectorGradientIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorDomainLFKernel creates an mfem::VectorDomainLFIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorDomainLFKernel)
{
  mfem::Vector expected1({2.0, 1.0, 0.0});

  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorDomainLFKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") =
      std::to_string(expected1[0]) + " " + std::to_string(expected1[1]) + " " +
      std::to_string(expected1[2]);
  MFEMVectorDomainLFKernel & kernel =
      addObject<MFEMVectorDomainLFKernel>("MFEMVectorDomainLFKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorDomainLFIntegrator *>(kernel.createLFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorFEDomainLFKernel creates an mfem::VectorFEDomainLFIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorFEDomainLFKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorFEDomainLFKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  MFEMVectorFEDomainLFKernel & kernel =
      addObject<MFEMVectorFEDomainLFKernel>("MFEMVectorFEDomainLFKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorFEDomainLFIntegrator *>(kernel.createLFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorFEMassKernel creates an mfem::VectorFEMassIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorFEMassKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorFEMassKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  MFEMVectorFEMassKernel & kernel =
      addObject<MFEMVectorFEMassKernel>("MFEMVectorFEMassKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorFEMassIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorFEWeakDivergenceKernel creates an mfem::VectorFEWeakDivergenceIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorFEWeakDivergenceKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorFEWeakDivergenceKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  MFEMVectorFEWeakDivergenceKernel & kernel = addObject<MFEMVectorFEWeakDivergenceKernel>(
      "MFEMVectorFEWeakDivergenceKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::VectorFEWeakDivergenceIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarCurlKernel creates an mfem::MixedScalarCurlIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarCurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  MFEMMixedScalarCurlKernel & kernel =
      addObject<MFEMMixedScalarCurlKernel>("MFEMMixedScalarCurlKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedScalarCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

TEST_F(MFEMKernelTest, RejectsOffDiagonalNonlinearKernelWhenBuildingEquationSystem)
{
  InputParameters diag_test_params = _factory.getValidParams("MFEMDiffusionKernel");
  diag_test_params.set<VariableName>("variable") = "test_variable_name";

  InputParameters nonlinear_params = _factory.getValidParams("TestOffDiagonalNonlinearKernel");
  nonlinear_params.set<VariableName>("variable") = "test_variable_name";
  nonlinear_params.set<VariableName>("trial_variable") = "trial_variable_name";

  auto diag_test =
      addSharedObject<MFEMDiffusionKernel>("MFEMDiffusionKernel", "diag_test", diag_test_params);
  auto nonlinear = addSharedObject<TestOffDiagonalNonlinearKernel>(
      "TestOffDiagonalNonlinearKernel", "nonlinear_offdiag", nonlinear_params);

  TestEquationSystem eqn_system;
  eqn_system.AddKernel(diag_test);
  eqn_system.AddKernel(nonlinear);
  eqn_system.SetGradientRequired(true);

  try
  {
    eqn_system.initAndBuild(_mfem_problem->getProblemData().gridfunctions,
                            _mfem_problem->getProblemData().cmplx_gridfunctions,
                            mfem::AssemblyLevel::LEGACY);
    FAIL() << "Expected off-diagonal nonlinear MFEM kernel to be rejected";
  }
  catch (const std::runtime_error & error)
  {
    const std::string message(error.what());
    EXPECT_TRUE(message.find("off-diagonal MFEM nonlinear domain integrators") !=
                std::string::npos);
    EXPECT_TRUE(message.find("requires a gradient") != std::string::npos);
  }
}

TEST_F(MFEMKernelTest, AcceptsLinearOffDiagonalKernelWhenBuildingEquationSystem)
{
  InputParameters diag_test_params = _factory.getValidParams("MFEMDiffusionKernel");
  diag_test_params.set<VariableName>("variable") = "test_variable_name";

  InputParameters diag_trial_params = _factory.getValidParams("MFEMDiffusionKernel");
  diag_trial_params.set<VariableName>("variable") = "trial_variable_name";

  InputParameters linear_params = _factory.getValidParams("TestOffDiagonalLinearKernel");
  linear_params.set<VariableName>("variable") = "test_variable_name";
  linear_params.set<VariableName>("trial_variable") = "trial_variable_name";

  auto diag_test =
      addSharedObject<MFEMDiffusionKernel>("MFEMDiffusionKernel", "diag_test", diag_test_params);
  auto diag_trial =
      addSharedObject<MFEMDiffusionKernel>("MFEMDiffusionKernel", "diag_trial", diag_trial_params);
  auto linear = addSharedObject<TestOffDiagonalLinearKernel>(
      "TestOffDiagonalLinearKernel", "linear_offdiag", linear_params);

  TestEquationSystem eqn_system;
  eqn_system.AddKernel(diag_test);
  // Keep a diagonal contribution on the trial variable so this exercises the supported mixed
  // 2x2 system path rather than a case where trial_variable_name is only an eliminated coupling.
  eqn_system.AddKernel(diag_trial);
  eqn_system.AddKernel(linear);

  EXPECT_NO_THROW(eqn_system.initAndBuild(_mfem_problem->getProblemData().gridfunctions,
                                          _mfem_problem->getProblemData().cmplx_gridfunctions,
                                          mfem::AssemblyLevel::LEGACY));
}

TEST_F(MFEMKernelTest, RejectsGetGradientForModernAssemblyWhenGradientIsRequired)
{
  InputParameters linear_params = _factory.getValidParams("MFEMDiffusionKernel");
  linear_params.set<VariableName>("variable") = "test_variable_name";

  InputParameters nonlinear_params = _factory.getValidParams("TestDiagonalNonlinearKernel");
  nonlinear_params.set<VariableName>("variable") = "test_variable_name";

  auto linear =
      addSharedObject<MFEMDiffusionKernel>("MFEMDiffusionKernel", "diag_linear", linear_params);
  auto nonlinear = addSharedObject<TestDiagonalNonlinearKernel>(
      "TestDiagonalNonlinearKernel", "diag_nonlinear", nonlinear_params);

  TestEquationSystem eqn_system;
  eqn_system.AddKernel(linear);
  eqn_system.AddKernel(nonlinear);
  eqn_system.SetGradientRequired(true);
  eqn_system.initAndBuild(_mfem_problem->getProblemData().gridfunctions,
                          _mfem_problem->getProblemData().cmplx_gridfunctions,
                          mfem::AssemblyLevel::LEGACY);
  eqn_system.setAssemblyLevel(mfem::AssemblyLevel::FULL);

  // The GetGradient() guard fires before the vector argument is accessed, so a
  // default-constructed (empty) vector is sufficient to reach it. Skipping
  // FormSystem avoids putting _h_blocks and _linear_operator into the
  // mixed-level state that caused teardown failures in earlier iterations of
  // this test, allowing eqn_system to be stack-allocated and destroyed normally.
  const mfem::Vector dummy;
  try
  {
    eqn_system.GetGradient(dummy);
    FAIL() << "Expected GetGradient to reject modern assembly when a gradient is required";
  }
  catch (const std::runtime_error & error)
  {
    const std::string message(error.what());
    EXPECT_TRUE(message.find("require GetGradient()") != std::string::npos);
    EXPECT_TRUE(message.find("require legacy assembly") != std::string::npos);
  }
}
/**
 * Test MFEMMixedGradGradKernel creates an mfem::MixedGradGradIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedGradGradKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedGradGradKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  MFEMMixedGradGradKernel & kernel =
      addObject<MFEMMixedGradGradKernel>("MFEMMixedGradGradKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedGradGradIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarWeakCurlKernel creates an mfem::MixedScalarWeakCurlIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarWeakCurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarWeakCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  MFEMMixedScalarWeakCurlKernel & kernel = addObject<MFEMMixedScalarWeakCurlKernel>(
      "MFEMMixedScalarWeakCurlKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedScalarWeakCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedVectorMassKernel creates an mfem::MixedVectorMassIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedVectorMassKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedVectorMassKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  MFEMMixedVectorMassKernel & kernel =
      addObject<MFEMMixedVectorMassKernel>("MFEMMixedVectorMassKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedVectorMassIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedVectorWeakDivergenceKernel creates an mfem::MixedVectorWeakDivergenceIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedVectorWeakDivergenceKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedVectorWeakDivergenceKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  MFEMMixedVectorWeakDivergenceKernel & kernel = addObject<MFEMMixedVectorWeakDivergenceKernel>(
      "MFEMMixedVectorWeakDivergenceKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedVectorWeakDivergenceIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMDomainLFGradKernel creates an mfem::DomainLFGradIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMDomainLFGradKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMDomainLFGradKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel =
      addObject<MFEMDomainLFGradKernel>("MFEMDomainLFGradKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::DomainLFGradIntegrator *>(kernel.createLFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorDomainLFGradKernel creates an mfem::VectorDomainLFGradIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorDomainLFGradKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorDomainLFGradKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3. 4. 5. 6. 7. 8. 9.";
  auto & kernel = addObject<MFEMVectorDomainLFGradKernel>(
      "MFEMVectorDomainLFGradKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorDomainLFGradIntegrator *>(kernel.createLFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorFEDomainLFCurlKernel creates an mfem::VectorFEDomainLFCurlIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorFEDomainLFCurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorFEDomainLFCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMVectorFEDomainLFCurlKernel>(
      "MFEMVectorFEDomainLFCurlKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::VectorFEDomainLFCurlIntegrator *>(kernel.createLFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorFEDomainLFDivKernel creates an mfem::VectorFEDomainLFDivIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorFEDomainLFDivKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorFEDomainLFDivKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel = addObject<MFEMVectorFEDomainLFDivKernel>(
      "MFEMVectorFEDomainLFDivKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::VectorFEDomainLFDivIntegrator *>(kernel.createLFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMWhiteGaussianNoiseDomainLFKernel creates an mfem::WhiteGaussianNoiseDomainLFIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMWhiteGaussianNoiseDomainLFKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMWhiteGaussianNoiseDomainLFKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<unsigned int>("seed") = 5;
  auto & kernel = addObject<MFEMWhiteGaussianNoiseDomainLFKernel>(
      "MFEMWhiteGaussianNoiseDomainLFKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::WhiteGaussianNoiseDomainLFIntegrator *>(kernel.createLFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMConvectionKernel creates an mfem::ConvectionIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMConvectionKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMConvectionKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMConvectionKernel>("MFEMConvectionKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::ConvectionIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMConservativeConvectionKernel creates an mfem::ConservativeConvectionIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMConservativeConvectionKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMConservativeConvectionKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMConservativeConvectionKernel>(
      "MFEMConservativeConvectionKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::ConservativeConvectionIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMGroupConvectionKernel creates an mfem::GroupConvectionIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMGroupConvectionKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMGroupConvectionKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel =
      addObject<MFEMGroupConvectionKernel>("MFEMGroupConvectionKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::GroupConvectionIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorCurlCurlKernel creates an mfem::VectorCurlCurlIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorCurlCurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorCurlCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel =
      addObject<MFEMVectorCurlCurlKernel>("MFEMVectorCurlCurlKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorCurlCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorMassKernel creates an mfem::VectorMassIntegrator from a scalar coefficient
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorMassKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorMassKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel = addObject<MFEMVectorMassKernel>("MFEMVectorMassKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorMassIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorMassKernel creates an mfem::VectorMassIntegrator from a vector coefficient
 * successfully, and rejects a scalar coefficient set alongside it.
 */
TEST_F(MFEMKernelTest, MFEMVectorMassKernelVectorCoefficient)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorMassKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMVectorMassKernel>("MFEMVectorMassKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorMassIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;

  // Check for failure if both the scalar and the vector coefficient are set
  InputParameters both_params = _factory.getValidParams("MFEMVectorMassKernel");
  both_params.set<VariableName>("variable") = "test_variable_name";
  both_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  both_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  EXPECT_THROW(addObject<MFEMVectorMassKernel>("MFEMVectorMassKernel", "kernel2", both_params),
               std::runtime_error);
}

/**
 * Test MFEMVectorDiffusionKernel creates an mfem::VectorDiffusionIntegrator from a scalar
 * coefficient successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorDiffusionKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorDiffusionKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel =
      addObject<MFEMVectorDiffusionKernel>("MFEMVectorDiffusionKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorDiffusionIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorDiffusionKernel creates an mfem::VectorDiffusionIntegrator from a vector
 * coefficient successfully, and rejects a scalar coefficient set alongside it.
 */
TEST_F(MFEMKernelTest, MFEMVectorDiffusionKernelVectorCoefficient)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorDiffusionKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel =
      addObject<MFEMVectorDiffusionKernel>("MFEMVectorDiffusionKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorDiffusionIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;

  // Check for failure if both the scalar and the vector coefficient are set
  InputParameters both_params = _factory.getValidParams("MFEMVectorDiffusionKernel");
  both_params.set<VariableName>("variable") = "test_variable_name";
  both_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  both_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  EXPECT_THROW(
      addObject<MFEMVectorDiffusionKernel>("MFEMVectorDiffusionKernel", "kernel2", both_params),
      std::runtime_error);
}

/**
 * Test MFEMDGTraceKernel creates an mfem::DGTraceIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMDGTraceKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMDGTraceKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMDGTraceKernel>("MFEMDGTraceKernel", "kernel1", kernel_params);

  // Test MFEMKernel is applied to interior faces
  EXPECT_TRUE(kernel.isDGKernel());

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::DGTraceIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMNonconservativeDGTraceKernel creates an mfem::NonconservativeDGTraceIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMNonconservativeDGTraceKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMNonconservativeDGTraceKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  kernel_params.set<mfem::real_t>("alpha") = -1.0;
  kernel_params.set<mfem::real_t>("beta") = -0.5;
  auto & kernel = addObject<MFEMNonconservativeDGTraceKernel>(
      "MFEMNonconservativeDGTraceKernel", "kernel1", kernel_params);

  // Test MFEMKernel is applied to interior faces
  EXPECT_TRUE(kernel.isDGKernel());

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::NonconservativeDGTraceIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMDGDiffusionBR2Kernel creates an mfem::DGDiffusionBR2Integrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMDGDiffusionBR2Kernel)
{
  // The BR2 integrator requires a DG space, so register a variable on an L2 space
  auto pm = _mfem_mesh_ptr->getMFEMParMeshPtr().get();
  auto * fec = new mfem::L2_FECollection(1, pm->Dimension());
  auto gf = std::make_shared<mfem::ParGridFunction>(new mfem::ParFiniteElementSpace(pm, fec));
  // Transfer ownership of the collection and the space to the grid function
  gf->MakeOwner(fec);
  _mfem_problem->getProblemData().gridfunctions.Register("dg_variable_name", gf);

  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMDGDiffusionBR2Kernel");
  kernel_params.set<VariableName>("variable") = "dg_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  kernel_params.set<mfem::real_t>("eta") = 1.5;
  auto & kernel =
      addObject<MFEMDGDiffusionBR2Kernel>("MFEMDGDiffusionBR2Kernel", "kernel1", kernel_params);

  // Test MFEMKernel is applied to interior faces
  EXPECT_TRUE(kernel.isDGKernel());

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::DGDiffusionBR2Integrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMDGElasticityKernel creates an mfem::DGElasticityIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMDGElasticityKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMDGElasticityKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("lambda") = "2.0";
  kernel_params.set<MFEMScalarCoefficientName>("mu") = "3.0";
  auto & kernel =
      addObject<MFEMDGElasticityKernel>("MFEMDGElasticityKernel", "kernel1", kernel_params);

  // Test MFEMKernel is applied to interior faces
  EXPECT_TRUE(kernel.isDGKernel());

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::DGElasticityIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMGradientKernel creates an mfem::GradientIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMGradientKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMGradientKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel = addObject<MFEMGradientKernel>("MFEMGradientKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::GradientIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorFECurlKernel creates an mfem::VectorFECurlIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorFECurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorFECurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel =
      addObject<MFEMVectorFECurlKernel>("MFEMVectorFECurlKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorFECurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMDerivativeKernel creates an mfem::DerivativeIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMDerivativeKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMDerivativeKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  kernel_params.set<unsigned int>("component") = 1;
  auto & kernel = addObject<MFEMDerivativeKernel>("MFEMDerivativeKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::DerivativeIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedCurlKernel creates an mfem::MixedCurlIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedCurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel = addObject<MFEMMixedCurlKernel>("MFEMMixedCurlKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMVectorDivergenceKernel creates an mfem::VectorDivergenceIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMVectorDivergenceKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMVectorDivergenceKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel =
      addObject<MFEMVectorDivergenceKernel>("MFEMVectorDivergenceKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::VectorDivergenceIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarMassKernel creates an mfem::MixedScalarMassIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarMassKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarMassKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel =
      addObject<MFEMMixedScalarMassKernel>("MFEMMixedScalarMassKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedScalarMassIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarDerivativeKernel creates an mfem::MixedScalarDerivativeIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarDerivativeKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarDerivativeKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel = addObject<MFEMMixedScalarDerivativeKernel>(
      "MFEMMixedScalarDerivativeKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedScalarDerivativeIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarWeakDerivativeKernel creates an mfem::MixedScalarWeakDerivativeIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarWeakDerivativeKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarWeakDerivativeKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel = addObject<MFEMMixedScalarWeakDerivativeKernel>(
      "MFEMMixedScalarWeakDerivativeKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedScalarWeakDerivativeIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarDivergenceKernel creates an mfem::MixedScalarDivergenceIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarDivergenceKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarDivergenceKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel = addObject<MFEMMixedScalarDivergenceKernel>(
      "MFEMMixedScalarDivergenceKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedScalarDivergenceIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarWeakGradientKernel creates an mfem::MixedScalarWeakGradientIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarWeakGradientKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarWeakGradientKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel = addObject<MFEMMixedScalarWeakGradientKernel>(
      "MFEMMixedScalarWeakGradientKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedScalarWeakGradientIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedVectorProductKernel creates an mfem::MixedVectorProductIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedVectorProductKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedVectorProductKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMMixedVectorProductKernel>(
      "MFEMMixedVectorProductKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedVectorProductIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedVectorDivergenceKernel creates an mfem::MixedVectorDivergenceIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedVectorDivergenceKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedVectorDivergenceKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMMixedVectorDivergenceKernel>(
      "MFEMMixedVectorDivergenceKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedVectorDivergenceIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedCrossProductKernel creates an mfem::MixedCrossProductIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedCrossProductKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedCrossProductKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMMixedCrossProductKernel>(
      "MFEMMixedCrossProductKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedCrossProductIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedDotProductKernel creates an mfem::MixedDotProductIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedDotProductKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedDotProductKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel =
      addObject<MFEMMixedDotProductKernel>("MFEMMixedDotProductKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedDotProductIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedWeakGradDotKernel creates an mfem::MixedWeakGradDotIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedWeakGradDotKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedWeakGradDotKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel =
      addObject<MFEMMixedWeakGradDotKernel>("MFEMMixedWeakGradDotKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedWeakGradDotIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedWeakDivCrossKernel creates an mfem::MixedWeakDivCrossIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedWeakDivCrossKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedWeakDivCrossKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMMixedWeakDivCrossKernel>(
      "MFEMMixedWeakDivCrossKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedWeakDivCrossIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedCrossGradGradKernel creates an mfem::MixedCrossGradGradIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedCrossGradGradKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedCrossGradGradKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMMixedCrossGradGradKernel>(
      "MFEMMixedCrossGradGradKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedCrossGradGradIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedCrossCurlCurlKernel creates an mfem::MixedCrossCurlCurlIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedCrossCurlCurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedCrossCurlCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMMixedCrossCurlCurlKernel>(
      "MFEMMixedCrossCurlCurlKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedCrossCurlCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedCrossCurlGradKernel creates an mfem::MixedCrossCurlGradIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedCrossCurlGradKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedCrossCurlGradKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMMixedCrossCurlGradKernel>(
      "MFEMMixedCrossCurlGradKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedCrossCurlGradIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedCrossGradCurlKernel creates an mfem::MixedCrossGradCurlIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedCrossGradCurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedCrossGradCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMMixedCrossGradCurlKernel>(
      "MFEMMixedCrossGradCurlKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedCrossGradCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedWeakCurlCrossKernel creates an mfem::MixedWeakCurlCrossIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedWeakCurlCrossKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedWeakCurlCrossKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMMixedWeakCurlCrossKernel>(
      "MFEMMixedWeakCurlCrossKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedWeakCurlCrossIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarWeakCurlCrossKernel creates an mfem::MixedScalarWeakCurlCrossIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarWeakCurlCrossKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarWeakCurlCrossKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2.";
  auto & kernel = addObject<MFEMMixedScalarWeakCurlCrossKernel>(
      "MFEMMixedScalarWeakCurlCrossKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedScalarWeakCurlCrossIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedCrossGradKernel creates an mfem::MixedCrossGradIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedCrossGradKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedCrossGradKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel =
      addObject<MFEMMixedCrossGradKernel>("MFEMMixedCrossGradKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedCrossGradIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedCrossCurlKernel creates an mfem::MixedCrossCurlIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedCrossCurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedCrossCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel =
      addObject<MFEMMixedCrossCurlKernel>("MFEMMixedCrossCurlKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedCrossCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarCrossCurlKernel creates an mfem::MixedScalarCrossCurlIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarCrossCurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarCrossCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2.";
  auto & kernel = addObject<MFEMMixedScalarCrossCurlKernel>(
      "MFEMMixedScalarCrossCurlKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedScalarCrossCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarCrossGradKernel creates an mfem::MixedScalarCrossGradIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarCrossGradKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarCrossGradKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2.";
  auto & kernel = addObject<MFEMMixedScalarCrossGradKernel>(
      "MFEMMixedScalarCrossGradKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedScalarCrossGradIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarCrossProductKernel creates an mfem::MixedScalarCrossProductIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarCrossProductKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarCrossProductKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2.";
  auto & kernel = addObject<MFEMMixedScalarCrossProductKernel>(
      "MFEMMixedScalarCrossProductKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedScalarCrossProductIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarWeakCrossProductKernel creates an
 * mfem::MixedScalarWeakCrossProductIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarWeakCrossProductKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarWeakCrossProductKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2.";
  auto & kernel = addObject<MFEMMixedScalarWeakCrossProductKernel>(
      "MFEMMixedScalarWeakCrossProductKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedScalarWeakCrossProductIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedDirectionalDerivativeKernel creates an mfem::MixedDirectionalDerivativeIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedDirectionalDerivativeKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedDirectionalDerivativeKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMMixedDirectionalDerivativeKernel>(
      "MFEMMixedDirectionalDerivativeKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedDirectionalDerivativeIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedGradDivKernel creates an mfem::MixedGradDivIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedGradDivKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedGradDivKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel =
      addObject<MFEMMixedGradDivKernel>("MFEMMixedGradDivKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedGradDivIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedDivGradKernel creates an mfem::MixedDivGradIntegrator successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedDivGradKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedDivGradKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel =
      addObject<MFEMMixedDivGradKernel>("MFEMMixedDivGradKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedDivGradIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedScalarWeakDivergenceKernel creates an mfem::MixedScalarWeakDivergenceIntegrator
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedScalarWeakDivergenceKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedScalarWeakDivergenceKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMMixedScalarWeakDivergenceKernel>(
      "MFEMMixedScalarWeakDivergenceKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedScalarWeakDivergenceIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedCurlCurlKernel creates an mfem::MixedCurlCurlIntegrator from a scalar coefficient
 * successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedCurlCurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedCurlCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel =
      addObject<MFEMMixedCurlCurlKernel>("MFEMMixedCurlCurlKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedCurlCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedCurlCurlKernel creates an mfem::MixedCurlCurlIntegrator from a vector coefficient
 * successfully, and rejects a scalar coefficient set alongside it.
 */
TEST_F(MFEMKernelTest, MFEMMixedCurlCurlKernelVectorCoefficient)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedCurlCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel =
      addObject<MFEMMixedCurlCurlKernel>("MFEMMixedCurlCurlKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedCurlCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;

  // Check for failure if both the scalar and the vector coefficient are set
  InputParameters both_params = _factory.getValidParams("MFEMMixedCurlCurlKernel");
  both_params.set<VariableName>("variable") = "test_variable_name";
  both_params.set<VariableName>("trial_variable") = "trial_variable_name";
  both_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  both_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  EXPECT_THROW(
      addObject<MFEMMixedCurlCurlKernel>("MFEMMixedCurlCurlKernel", "kernel2", both_params),
      std::runtime_error);
}

/**
 * Test MFEMMixedVectorCurlKernel creates an mfem::MixedVectorCurlIntegrator from a scalar
 * coefficient successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedVectorCurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedVectorCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel =
      addObject<MFEMMixedVectorCurlKernel>("MFEMMixedVectorCurlKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedVectorCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedVectorCurlKernel creates an mfem::MixedVectorCurlIntegrator from a vector
 * coefficient successfully, and rejects a scalar coefficient set alongside it.
 */
TEST_F(MFEMKernelTest, MFEMMixedVectorCurlKernelVectorCoefficient)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedVectorCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel =
      addObject<MFEMMixedVectorCurlKernel>("MFEMMixedVectorCurlKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator = dynamic_cast<mfem::MixedVectorCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;

  // Check for failure if both the scalar and the vector coefficient are set
  InputParameters both_params = _factory.getValidParams("MFEMMixedVectorCurlKernel");
  both_params.set<VariableName>("variable") = "test_variable_name";
  both_params.set<VariableName>("trial_variable") = "trial_variable_name";
  both_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  both_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  EXPECT_THROW(
      addObject<MFEMMixedVectorCurlKernel>("MFEMMixedVectorCurlKernel", "kernel2", both_params),
      std::runtime_error);
}

/**
 * Test MFEMMixedVectorWeakCurlKernel creates an mfem::MixedVectorWeakCurlIntegrator from a scalar
 * coefficient successfully.
 */
TEST_F(MFEMKernelTest, MFEMMixedVectorWeakCurlKernel)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedVectorWeakCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  auto & kernel = addObject<MFEMMixedVectorWeakCurlKernel>(
      "MFEMMixedVectorWeakCurlKernel", "kernel1", kernel_params);

  // Test the trial variable name is different from the test variable name
  const std::string trial_name = kernel.getTrialVariableName();
  EXPECT_NE(trial_name, kernel.getTestVariableName());
  EXPECT_EQ(trial_name, "trial_variable_name");

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedVectorWeakCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;
}

/**
 * Test MFEMMixedVectorWeakCurlKernel creates an mfem::MixedVectorWeakCurlIntegrator from a vector
 * coefficient successfully, and rejects a scalar coefficient set alongside it.
 */
TEST_F(MFEMKernelTest, MFEMMixedVectorWeakCurlKernelVectorCoefficient)
{
  // Construct kernel
  InputParameters kernel_params = _factory.getValidParams("MFEMMixedVectorWeakCurlKernel");
  kernel_params.set<VariableName>("variable") = "test_variable_name";
  kernel_params.set<VariableName>("trial_variable") = "trial_variable_name";
  kernel_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  auto & kernel = addObject<MFEMMixedVectorWeakCurlKernel>(
      "MFEMMixedVectorWeakCurlKernel", "kernel1", kernel_params);

  // Test MFEMKernel returns an integrator of the expected type
  auto integrator =
      dynamic_cast<mfem::MixedVectorWeakCurlIntegrator *>(kernel.createBFIntegrator());
  ASSERT_NE(integrator, nullptr);
  delete integrator;

  // Check for failure if both the scalar and the vector coefficient are set
  InputParameters both_params = _factory.getValidParams("MFEMMixedVectorWeakCurlKernel");
  both_params.set<VariableName>("variable") = "test_variable_name";
  both_params.set<VariableName>("trial_variable") = "trial_variable_name";
  both_params.set<MFEMScalarCoefficientName>("coefficient") = "2.0";
  both_params.set<MFEMVectorCoefficientName>("vector_coefficient") = "1. 2. 3.";
  EXPECT_THROW(addObject<MFEMMixedVectorWeakCurlKernel>(
                   "MFEMMixedVectorWeakCurlKernel", "kernel2", both_params),
               std::runtime_error);
}

#endif
