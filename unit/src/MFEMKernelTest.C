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
#include "MFEMCurlCurlKernel.h"
#include "MFEMDiffusionKernel.h"
#include "MFEMDivDivKernel.h"
#include "MFEMLinearElasticityKernel.h"
#include "MFEMMixedBilinearFormKernel.h"
#include "MFEMMixedScalarCurlKernel.h"
#include "MFEMMixedVectorGradientKernel.h"
#include "MFEMVectorDomainLFKernel.h"
#include "MFEMVectorFEDomainLFKernel.h"
#include "MFEMVectorFEMassKernel.h"
#include "MFEMVectorFEWeakDivergenceKernel.h"

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

registerMooseObject("MooseUnitApp", TestOffDiagonalLinearKernel);
registerMooseObject("MooseUnitApp", TestOffDiagonalNonlinearKernel);

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
  diag_test_params.set<MFEMScalarCoefficientName>("coefficient") = "1.0";

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

  try
  {
    eqn_system.initAndBuild(_mfem_problem->getProblemData().gridfunctions,
                            _mfem_problem->getProblemData().cmplx_gridfunctions,
                            mfem::AssemblyLevel::LEGACY);
    FAIL() << "Expected off-diagonal nonlinear MFEM kernel to be rejected";
  }
  catch (const std::runtime_error & error)
  {
    EXPECT_TRUE(std::string(error.what()).find("not currently implemented") != std::string::npos);
  }
}

TEST_F(MFEMKernelTest, AcceptsLinearOffDiagonalKernelWhenBuildingEquationSystem)
{
  InputParameters diag_test_params = _factory.getValidParams("MFEMDiffusionKernel");
  diag_test_params.set<VariableName>("variable") = "test_variable_name";
  diag_test_params.set<MFEMScalarCoefficientName>("coefficient") = "1.0";

  InputParameters diag_trial_params = _factory.getValidParams("MFEMDiffusionKernel");
  diag_trial_params.set<VariableName>("variable") = "trial_variable_name";
  diag_trial_params.set<MFEMScalarCoefficientName>("coefficient") = "1.0";

  InputParameters linear_params = _factory.getValidParams("TestOffDiagonalLinearKernel");
  linear_params.set<VariableName>("variable") = "test_variable_name";
  linear_params.set<VariableName>("trial_variable") = "trial_variable_name";

  auto diag_test =
      addSharedObject<MFEMDiffusionKernel>("MFEMDiffusionKernel", "diag_test_2", diag_test_params);
  auto diag_trial = addSharedObject<MFEMDiffusionKernel>(
      "MFEMDiffusionKernel", "diag_trial_2", diag_trial_params);
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

#endif
